#include "backend.h"

#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_output.h>

#include "compositor.h"
#include "keyboard.h"
#include "output.h"
#include "pointer.h"
#include "seat.h"
#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"
#include "zen/server.h"
#include "zen/wlr/render/glew.h"

static void zn_default_backend_destroy(struct zn_default_backend *self);

struct zn_default_backend *
zn_default_backend_get(struct zn_backend *base)
{
  struct zn_default_backend *self = zn_container_of(base, self, base);
  return self;
}

void
zn_default_backend_update_capabilities(struct zn_default_backend *self UNUSED)
{
  struct zn_server *server = zn_server_get_singleton();

  uint32_t capabilities = 0;
  struct zn_input_device_base *device = NULL;
  wl_list_for_each (device, &self->input_device_list, link) {
    switch (device->wlr_input_device->type) {
      case WLR_INPUT_DEVICE_KEYBOARD:
        capabilities |= WL_SEAT_CAPABILITY_KEYBOARD;
        break;
      case WLR_INPUT_DEVICE_POINTER:
        capabilities |= WL_SEAT_CAPABILITY_POINTER;
        break;
      case WLR_INPUT_DEVICE_TOUCH:
      case WLR_INPUT_DEVICE_TABLET_TOOL:
      case WLR_INPUT_DEVICE_TABLET_PAD:
      case WLR_INPUT_DEVICE_SWITCH:
        break;
    }
  }

  zn_seat_notify_capabilities(server->seat, capabilities);
}

void
zn_default_backend_notify_view_mapped(
    struct zn_default_backend *self, struct zn_view *view)
{
  wl_signal_emit(&self->base.events.view_mapped, view);
}

static void
zn_default_backend_handle_new_input(struct wl_listener *listener, void *data)
{
  struct zn_default_backend *self =
      zn_container_of(listener, self, new_input_listener);
  struct wlr_input_device *input_device = data;

  switch (input_device->type) {
    case WLR_INPUT_DEVICE_KEYBOARD: {
      struct zn_keyboard *keyboard = zn_keyboard_create(input_device);
      if (keyboard == NULL) {
        zn_warn("Failed to create a keyboard object");
      } else {
        wl_list_insert(&self->input_device_list, &keyboard->base.link);
      }
      break;
    }
    case WLR_INPUT_DEVICE_POINTER: {
      struct zn_pointer *pointer = zn_pointer_create(input_device);
      if (pointer == NULL) {
        zn_warn("Failed to create a pointer object");
      } else {
        wl_list_insert(&self->input_device_list, &pointer->base.link);
      }
      break;
    }
    case WLR_INPUT_DEVICE_TOUCH:        // fall through
    case WLR_INPUT_DEVICE_TABLET_TOOL:  // fall through
    case WLR_INPUT_DEVICE_TABLET_PAD:   // fall through
    case WLR_INPUT_DEVICE_SWITCH:
      break;
  }

  zn_default_backend_update_capabilities(self);
}

static void
zn_default_backend_handle_new_output(struct wl_listener *listener, void *data)
{
  struct zn_default_backend *self =
      zn_container_of(listener, self, new_output_listener);
  struct wlr_output *wlr_output = data;

  zn_debug("New output %p: %s (non-desktop: %d)", (void *)wlr_output,
      wlr_output->name, wlr_output->non_desktop);

  if (wlr_output->non_desktop) {
    zn_debug("Skip non-desktop output");
    // TODO(@Aki-7): DRM lease support
    return;
  }

  if (!wlr_output_init_render(
          wlr_output, self->wlr_allocator, self->wlr_renderer)) {
    zn_abort("Failed to initialize output rendering subsystem");
    return;
  }

  struct zn_output *output = zn_output_create(wlr_output);
  if (output == NULL) {
    zn_abort("Failed to create a zn_output");
    return;
  }

  wlr_output_layout_add(
      self->compositor->output_layout, output->wlr_output, 0, 0);

  wl_signal_emit(&self->base.events.new_screen, output->screen);
}

static struct wlr_texture *
zn_default_backend_create_wlr_texture_from_pixels(struct zn_backend *base,
    uint32_t format, uint32_t stride, uint32_t width, uint32_t height,
    const void *data)
{
  struct zn_default_backend *self = zn_container_of(base, self, base);
  if (!zn_assert(self->wlr_backend, "zn_backend has already stopped")) {
    return NULL;
  }

  return wlr_texture_from_pixels(
      self->wlr_renderer, format, stride, width, height, data);
}

static bool
zn_default_backend_start(struct zn_backend *base)
{
  struct zn_default_backend *self = zn_container_of(base, self, base);
  return wlr_backend_start(self->wlr_backend);
}

static void
zn_default_backend_stop(struct zn_backend *base)
{
  struct zn_default_backend *self = zn_container_of(base, self, base);
  wlr_backend_destroy(self->wlr_backend);
  self->wlr_backend = NULL;
}

static void
zn_default_backend_handle_destroy(struct zn_backend *base)
{
  struct zn_default_backend *self = zn_container_of(base, self, base);
  zn_default_backend_destroy(self);
}

static const struct zn_backend_interface implementation = {
    .create_wlr_texture_from_pixels =
        zn_default_backend_create_wlr_texture_from_pixels,
    .start = zn_default_backend_start,
    .stop = zn_default_backend_stop,
    .destroy = zn_default_backend_handle_destroy,
};

struct zn_backend *
zn_default_backend_create(struct wl_display *display, struct zn_seat *zn_seat)
{
  struct zn_default_backend *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wl_signal_init(&self->base.events.new_screen);
  wl_signal_init(&self->base.events.view_mapped);
  wl_signal_init(&self->base.events.destroy);
  self->base.impl = &implementation;
  self->display = display;
  wl_list_init(&self->input_device_list);

  self->wlr_backend = wlr_backend_autocreate(display);
  if (self->wlr_backend == NULL) {
    zn_error("Failed to create a wlr_backend");
    goto err_free;
  }

  int drm_fd = wlr_backend_get_drm_fd(self->wlr_backend);
  if (drm_fd < 0) {
    zn_error("Failed to get drm fd");
    goto err_wlr_backend;
  }

  self->wlr_renderer = wlr_glew_renderer_create_with_drm_fd(drm_fd);
  if (self->wlr_renderer == NULL) {
    zn_error("Failed to create a wlr_glew_renderer");
    goto err_wlr_backend;
  }

  if (!wlr_renderer_init_wl_display(self->wlr_renderer, self->display)) {
    zn_error("Failed to initialize renderer with wl_display");
    goto err_wlr_renderer;
  }

  self->wlr_allocator =
      wlr_allocator_autocreate(self->wlr_backend, self->wlr_renderer);
  if (self->wlr_allocator == NULL) {
    zn_error("Failed to autocreate a wlr_allocator");
    goto err_wlr_renderer;
  }

  self->compositor = zn_compositor_create(display, self->wlr_renderer);
  if (self->compositor == NULL) {
    zn_error("Failed to create zn_compositor");
    goto err_wlr_allocator;
  }

  wlr_xwayland_set_seat(self->compositor->xwayland, zn_seat->wlr_seat);

  self->new_output_listener.notify = zn_default_backend_handle_new_output;
  wl_signal_add(
      &self->wlr_backend->events.new_output, &self->new_output_listener);

  self->new_input_listener.notify = zn_default_backend_handle_new_input;
  wl_signal_add(
      &self->wlr_backend->events.new_input, &self->new_input_listener);

  return &self->base;

err_wlr_allocator:
  wlr_allocator_destroy(self->wlr_allocator);

err_wlr_renderer:
  wlr_renderer_destroy(self->wlr_renderer);

err_wlr_backend:
  wlr_backend_destroy(self->wlr_backend);

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_default_backend_destroy(struct zn_default_backend *self)
{
  zn_signal_emit_mutable(&self->base.events.destroy, NULL);

  wl_list_remove(&self->new_input_listener.link);
  wl_list_remove(&self->new_output_listener.link);
  zn_compositor_destroy(self->compositor);
  wlr_allocator_destroy(self->wlr_allocator);
  wlr_renderer_destroy(self->wlr_renderer);
  if (self->wlr_backend) {
    wlr_backend_destroy(self->wlr_backend);
  }
  wl_list_remove(&self->input_device_list);
  wl_list_remove(&self->base.events.destroy.listener_list);
  wl_list_remove(&self->base.events.view_mapped.listener_list);
  wl_list_remove(&self->base.events.new_screen.listener_list);
  free(self);
}
