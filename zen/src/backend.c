#include "backend.h"

#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_output.h>

#include "backend/output.h"
#include "backend/pointer.h"
#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"
#include "zen/wlr/render/glew.h"

void
zn_backend_impl_update_capabilities(struct zn_backend_impl *self UNUSED)
{
  // TODO(@Aki-7): implement
}

static void
zn_backend_impl_handle_new_input(
    struct wl_listener *listener UNUSED, void *data UNUSED)
{
  struct zn_backend_impl *self =
      zn_container_of(listener, self, new_input_listener);
  struct wlr_input_device *input_device = data;

  switch (input_device->type) {
    case WLR_INPUT_DEVICE_KEYBOARD:
      break;
    case WLR_INPUT_DEVICE_POINTER: {
      struct zn_pointer *pointer = zn_pointer_create(self, input_device);
      wl_list_insert(&self->input_device_list, &pointer->base.link);
      break;
    }
    case WLR_INPUT_DEVICE_TOUCH:        // fall through
    case WLR_INPUT_DEVICE_TABLET_TOOL:  // fall through
    case WLR_INPUT_DEVICE_TABLET_PAD:   // fall through
    case WLR_INPUT_DEVICE_SWITCH:
      break;
  }

  zn_backend_impl_update_capabilities(self);
}

static void
zn_backend_impl_handle_new_output(struct wl_listener *listener, void *data)
{
  struct zn_backend_impl *self =
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

  wl_signal_emit(&self->base.events.new_screen, output->screen);
}

struct zn_backend_impl *
zn_backend_impl_get(struct zn_backend *base)
{
  struct zn_backend_impl *self = zn_container_of(base, self, base);
  return self;
}

bool
zn_backend_impl_start(struct zn_backend_impl *self)
{
  return wlr_backend_start(self->wlr_backend);
}

struct zn_backend_impl *
zn_backend_impl_create(struct wl_display *display)
{
  struct zn_backend_impl *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->display = display;
  wl_signal_init(&self->base.events.new_screen);
  wl_signal_init(&self->events.destroy);
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

  self->new_output_listener.notify = zn_backend_impl_handle_new_output;
  wl_signal_add(
      &self->wlr_backend->events.new_output, &self->new_output_listener);

  self->new_input_listener.notify = zn_backend_impl_handle_new_input;
  wl_signal_add(
      &self->wlr_backend->events.new_input, &self->new_input_listener);

  return self;

err_wlr_renderer:
  wlr_renderer_destroy(self->wlr_renderer);

err_wlr_backend:
  wlr_backend_destroy(self->wlr_backend);

err_free:
  free(self);

err:
  return NULL;
}

void
zn_backend_impl_destroy(struct zn_backend_impl *self)
{
  zn_signal_emit_mutable(&self->events.destroy, NULL);

  wl_list_remove(&self->new_input_listener.link);
  wl_list_remove(&self->new_output_listener.link);
  wlr_allocator_destroy(self->wlr_allocator);
  wlr_renderer_destroy(self->wlr_renderer);
  wlr_backend_destroy(self->wlr_backend);
  wl_list_remove(&self->events.destroy.listener_list);
  wl_list_remove(&self->input_device_list);
  wl_list_remove(&self->base.events.new_screen.listener_list);
  free(self);
}
