#include <libzen-compositor/libzen-compositor.h>
#include <stdio.h>
#include <wayland-server.h>
#include <zigen-server-protocol.h>

#include "virtual-object.h"

#define DEFAULT_REPAINT_WINDOW 7

static void
zen_compositor_protocol_create_virtual_object(
    struct wl_client *client, struct wl_resource *resource, uint32_t id)
{
  struct zen_compositor *compositor = wl_resource_get_user_data(resource);
  struct zen_virtual_object *virtual_object;

  virtual_object = zen_virtual_object_create(client, id, compositor);
  if (virtual_object == NULL) {
    zen_log("compositor: failed to create virtual object\n");
  }
}

static const struct zgn_compositor_interface compositor_interface = {
    .create_virtual_object = zen_compositor_protocol_create_virtual_object,
};

static void
zen_compositor_bind(
    struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
  struct zen_compositor *compositor = data;
  struct wl_resource *resource;

  resource = wl_resource_create(client, &zgn_compositor_interface, version, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    zen_log("compositor: failed to create a resource\n");
    return;
  }

  wl_resource_set_implementation(
      resource, &compositor_interface, compositor, NULL);
}

static int
repaint_timer_handler(void *data)
{
  uint32_t frame_time_msec;
  struct zen_compositor *compositor = data;
  struct zen_output *output = compositor->backend->output;

  frame_time_msec = timespec_to_msec(&output->frame_time);

  wl_signal_emit(&compositor->frame_signal, &frame_time_msec);
  wl_display_flush_clients(compositor->display);
  output->repaint(output);
  return 0;
}

WL_EXPORT int
zen_compositor_load_shell(struct zen_compositor *compositor)
{
  struct zen_shell_base *shell_base;

  shell_base = zen_shell_create(compositor);
  if (shell_base == NULL) {
    zen_log("compositor: failed to create a shell\n");
    goto err;
  }

  compositor->shell_base = shell_base;

  return 0;

err:
  return -1;
}

WL_EXPORT int
zen_compositor_load_renderer(struct zen_compositor *compositor)
{
  assert(compositor->shell_base && "load shell before backend");

  struct zen_renderer *renderer;

  renderer = zen_renderer_create(compositor);
  if (renderer == NULL) {
    zen_log("compositor: failed to create a renderer\n");
    goto err;
  }

  compositor->renderer = renderer;

  return 0;

err:
  return -1;
}

WL_EXPORT int
zen_compositor_load_backend(struct zen_compositor *compositor)
{
  assert(compositor->shell_base && "load shell before backend");
  assert(compositor->renderer && "load renderer before backend");

  struct zen_backend *backend;

  backend = zen_backend_create(compositor);
  if (backend == NULL) {
    zen_log("compositor: failed to create a backend\n");
    goto err;
  }

  compositor->backend = backend;

  return 0;

err:
  return -1;
}

WL_EXPORT struct zen_compositor *
zen_compositor_create(struct wl_display *display)
{
  struct zen_compositor *compositor;
  struct wl_event_loop *loop;
  struct wl_event_source *repaint_timer;
  struct wl_global *global;

  compositor = zalloc(sizeof *compositor);
  if (compositor == NULL) {
    zen_log("compositor: failed to allcate memory\n");
    goto err;
  }

  loop = wl_display_get_event_loop(display);
  repaint_timer =
      wl_event_loop_add_timer(loop, repaint_timer_handler, compositor);

  global = wl_global_create(
      display, &zgn_compositor_interface, 1, compositor, zen_compositor_bind);
  if (global == NULL) {
    zen_log("compositor: failed to create a compositor global\n");
    goto err_global;
  }

  if (wl_display_init_shm(display) != 0) {
    zen_log("compositor: failed to init shm\n");
    goto err_shm;
  }

  compositor->display = display;
  compositor->global = global;
  wl_signal_init(&compositor->frame_signal);
  compositor->backend = NULL;
  compositor->repaint_timer = repaint_timer;
  compositor->repaint_window_msec = DEFAULT_REPAINT_WINDOW;

  return compositor;

err_shm:
  wl_global_destroy(global);

err_global:
  free(compositor);

err:
  return NULL;
}

WL_EXPORT void
zen_compositor_destroy(struct zen_compositor *compositor)
{
  if (compositor->backend) zen_backend_destroy(compositor->backend);
  if (compositor->shell_base) zen_shell_destroy(compositor->shell_base);
  if (compositor->renderer) zen_renderer_destroy(compositor->renderer);
  wl_event_source_remove(compositor->repaint_timer);
  wl_global_destroy(compositor->global);
  free(compositor);
}

WL_EXPORT void
zen_compositor_finish_frame(
    struct zen_compositor *compositor, struct timespec next_repaint)
{
  struct timespec now;
  int64_t next_msec;

  if (timespec_get(&now, TIME_UTC) < 0)
    zen_log("compositor: [WARNING] failed to get current time\n");
  next_msec = timespec_sub_to_msec(&next_repaint, &now);
  next_msec -= compositor->repaint_window_msec;

  if (next_msec < 1) next_msec = 1;

  wl_event_source_timer_update(compositor->repaint_timer, next_msec);
}
