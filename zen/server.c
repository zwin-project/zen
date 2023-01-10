#include "zen/server.h"

#include <stdio.h>
#include <wlr/render/glew.h>
#include <wlr/types/wlr_output.h>
#include <zen-common.h>

#include "zen/config/config-parser.h"
#include "zen/config/config.h"
#include "zen/cursor.h"
#include "zen/ray.h"
#include "zen/screen-layout.h"
#include "zen/screen/output.h"
#include "zen/virtual-object.h"

static struct zn_server *server_singleton = NULL;

static void
zn_server_handle_new_input(struct wl_listener *listener, void *data)
{
  struct zn_server *self = zn_container_of(listener, self, new_input_listener);
  struct wlr_input_device *wlr_input = data;
  zn_input_manager_handle_new_wlr_input(self->input_manager, wlr_input);
}

static void
zn_server_handle_new_output(struct wl_listener *listener, void *data)
{
  struct zn_server *self = zn_container_of(listener, self, new_output_listener);
  struct wlr_output *wlr_output = data;
  struct zn_output *output;

  zn_debug("New output %p: %s (non-desktop: %d)", (void *)wlr_output,
      wlr_output->name, wlr_output->non_desktop);

  if (wlr_output->non_desktop) {
    zn_debug("Skip non-desktop output");
    // TODO: DRM lease support for VR headsets
    return;
  }

  if (wlr_output_init_render(wlr_output, self->allocator, self->renderer) ==
      false) {
    zn_abort("Failed to initialize output renderer");
    return;
  }

  output = zn_output_create(wlr_output);
  if (output == NULL) {
    zn_error("Failed to create a zn_output");
    zn_terminate(EXIT_FAILURE);
    return;
  }

  zn_scene_new_screen(self->scene, output->screen);
}

static void
zn_server_handle_new_virtual_object(struct wl_listener *listener, void *data)
{
  struct zn_server *self =
      zn_container_of(listener, self, new_virtual_object_listener);
  struct zgnr_virtual_object *zgnr_virtual_object = data;

  (void)zn_virtual_object_create(zgnr_virtual_object);
}

struct zn_server *
zn_server_get_singleton(void)
{
  zn_assert(server_singleton != NULL,
      "zn_server_get_singleton was called before creating zn_server");
  return server_singleton;
}

void
zn_server_change_display_system(
    struct zn_server *self, enum zn_display_system_state display_system)
{
  struct zn_screen *screen;

  if (self->display_system == display_system) return;

  self->display_system = ZN_DISPLAY_SYSTEM_SCREEN;  // enable screen damage

  wl_list_for_each (screen, &self->scene->screen_layout->screen_list, link) {
    zn_screen_damage_whole(screen);
  }

  self->display_system = display_system;

  wl_signal_emit(&self->events.display_system_changed, &self->display_system);
}

/** returns exit code */
int
zn_server_run(struct zn_server *self)
{
  if (!wlr_backend_start(self->wlr_backend)) {
    zn_error("Failed to start backend");
    return EXIT_FAILURE;
  }

  self->default_space_app_pid =
      zn_launch_command(self->config->space_default_app);
  if (self->default_space_app_pid < 0) {
    zn_error("Failed to launch default space app");
    return EXIT_FAILURE;
  }

  if (!self->exitted) {
    wl_display_run(self->display);
  }

  return self->exit_code;
}

void
zn_server_terminate(struct zn_server *self, int exit_code)
{
  // FIXME: request session destruction, which results in display system change
  if (self->exitted) return;
  zn_server_change_display_system(self, ZN_DISPLAY_SYSTEM_SCREEN);
  self->exit_code = exit_code;
  self->exitted = true;
  wl_display_terminate(self->display);
}

struct zn_server *
zn_server_create(struct wl_display *display)
{
  struct zn_server *self;
  char socket_name_candidate[16];
  char *xdg;
  int drm_fd = -1;

  if (!zn_assert(server_singleton == NULL,
          "Tried to create zn_server multiple times")) {
    goto err;
  }

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  server_singleton = self;

  struct toml_table_t *config_table = zn_config_get_toml_table();
  self->config = zn_config_create(config_table);
  if (config_table != NULL) toml_free(config_table);
  if (self->config == NULL) {
    zn_error("Failed to create a config object");
    goto err_free;
  }

  self->display = display;
  self->exit_code = EXIT_FAILURE;
  self->exitted = false;
  self->loop = wl_display_get_event_loop(display);
  self->display_system = ZN_DISPLAY_SYSTEM_SCREEN;

  wl_signal_init(&self->events.display_system_changed);

  self->zgnr_backend = zgnr_backend_create(self->display);
  if (self->zgnr_backend == NULL) {
    zn_error("Failed to create a zgnr_backend");
    goto err_config;
  }

  self->wlr_backend = wlr_backend_autocreate(self->display);
  if (self->wlr_backend == NULL) {
    zn_error("Failed to create a wlr_backend");
    goto err_zgnr_backend;
  }

  drm_fd = wlr_backend_get_drm_fd(self->wlr_backend);
  if (drm_fd < 0) {
    zn_error("Failed to get drm fd");
    goto err_wlr_backend;
  }

  self->renderer = wlr_glew_renderer_create_with_drm_fd(drm_fd);
  if (self->renderer == NULL) {
    zn_error("Failed to create renderer");
    goto err_wlr_backend;
  }

  if (wlr_renderer_init_wl_display(self->renderer, self->display) == false) {
    zn_error("Failed to initialize renderer with wl_display");
    goto err_renderer;
  }

  self->allocator = wlr_allocator_autocreate(self->wlr_backend, self->renderer);
  if (self->allocator == NULL) {
    zn_error("Failed to create allocator");
    goto err_renderer;
  }

  self->screen_compositor =
      zn_screen_compositor_create(self->display, self->renderer);
  if (self->screen_compositor == NULL) {
    zn_error("Failed to create zn_screen_compositor");
    goto err_allocator;
  }

  self->appearance_system = zna_system_create(self->display);
  if (self->appearance_system == NULL) {
    zn_error("Failed to create a zna_system");
    goto err_screen_compositor;
  }

  self->scene = zn_scene_create();
  if (self->scene == NULL) {
    zn_error("Failed to create a zn_scene");
    goto err_appearance;
  }

  self->remote = zn_remote_create(self->display);
  if (self->remote == NULL) {
    zn_error("Failed to create a zn_remote");
    goto err_scene;
  }

  for (int i = 1; i <= 32; i++) {
    sprintf(socket_name_candidate, "wayland-%d", i);
    if (wl_display_add_socket(self->display, socket_name_candidate) >= 0) {
      self->socket = strdup(socket_name_candidate);
      break;
    }
  }

  if (self->socket == NULL) {
    zn_error("Failed to open wayland socket");
    goto err_remote;
  }

  setenv("WAYLAND_DISPLAY", self->socket, true);
  xdg = getenv("XDG_RUNTIME_DIR");
  zn_debug("WAYLAND_DISPLAY=%s", self->socket);
  zn_debug("XDG_RUNTIME_DIR=%s", xdg);

  self->shell = zn_shell_create(self->display, self->scene);
  if (self->shell == NULL) {
    zn_error("Failed to create zn_shell");
    goto err_socket;
  }

  zn_scene_initialize_boards(self->scene, self->config->board_initial_count);

  zn_ray_set_default_grab(
      self->scene->ray, zn_shell_get_default_grab(self->shell));

  self->input_manager = zn_input_manager_create(self->display);
  if (self->input_manager == NULL) {
    zn_error("Failed to create input manager");
    goto err_shell;
  }

  self->data_device_manager = zn_data_device_manager_create(self->display);
  if (self->data_device_manager == NULL) {
    zn_error("Failed to create data device manager");
    goto err_input_manager;
  }

  zn_scene_setup_keybindings(self->scene);

  self->new_input_listener.notify = zn_server_handle_new_input;
  wl_signal_add(
      &self->wlr_backend->events.new_input, &self->new_input_listener);

  self->new_output_listener.notify = zn_server_handle_new_output;
  wl_signal_add(
      &self->wlr_backend->events.new_output, &self->new_output_listener);

  self->new_virtual_object_listener.notify =
      zn_server_handle_new_virtual_object;
  wl_signal_add(&self->zgnr_backend->events.new_virtual_object,
      &self->new_virtual_object_listener);

  zgnr_backend_activate(self->zgnr_backend);

  return self;

err_input_manager:
  zn_input_manager_destroy(self->input_manager);

err_shell:
  zn_shell_destroy(self->shell);

err_socket:
  free(self->socket);

err_remote:
  zn_remote_destroy(self->remote);

err_scene:
  zn_scene_destroy(self->scene);

err_appearance:
  zna_system_destroy(self->appearance_system);

err_screen_compositor:
  zn_screen_compositor_destroy(self->screen_compositor);

err_allocator:
  wlr_allocator_destroy(self->allocator);

err_renderer:
  wlr_renderer_destroy(self->renderer);

err_wlr_backend:
  wlr_backend_destroy(self->wlr_backend);

err_zgnr_backend:
  zgnr_backend_destroy(self->zgnr_backend);

err_config:
  zn_config_destroy(self->config);

err_free:
  server_singleton = NULL;
  free(self);

err:
  return NULL;
}

void
zn_server_destroy_resources(struct zn_server *self)
{
  wlr_backend_destroy(self->wlr_backend);
  wl_display_destroy_clients(self->display);

  zn_scene_destroy_resources(self->scene);
}

void
zn_server_destroy(struct zn_server *self)
{
  zn_data_device_manager_destroy(self->data_device_manager);
  zn_input_manager_destroy(self->input_manager);
  zn_shell_destroy(self->shell);
  free(self->socket);
  zn_remote_destroy(self->remote);
  zn_scene_destroy(self->scene);
  zna_system_destroy(self->appearance_system);
  zn_screen_compositor_destroy(self->screen_compositor);
  wlr_allocator_destroy(self->allocator);
  wlr_renderer_destroy(self->renderer);
  zgnr_backend_destroy(self->zgnr_backend);
  zn_config_destroy(self->config);
  server_singleton = NULL;
  free(self);
}
