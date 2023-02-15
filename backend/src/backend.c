#include "zen/backend/default/backend.h"

#include <stdio.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/render/allocator.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_output.h>
#include <wlr/util/log.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/backend/default/output.h"
#include "zen/backend/default/seat.h"
#include "zen/backend/seat.h"
#include "zen/backend/wlr/render/glew.h"

#define DEFAULT_SEAT "seat0"

static struct zn_default_backend *
zn_default_backend_get(struct zn_backend *base)
{
  struct zn_default_backend *self = zn_container_of(base, self, base);
  return self;
}

static void
handle_wlr_log(
    enum wlr_log_importance wlr_importance, const char *fmt, va_list args)
{
  zn_log_importance_t importance = ZEN_DEBUG;

  switch (wlr_importance) {
    case WLR_ERROR:
      importance = ZEN_ERROR;
      break;
    case WLR_INFO:
      importance = ZEN_INFO;
      break;
    default:
      importance = ZEN_DEBUG;
      break;
  }

  int len = snprintf(NULL, 0, "[wlr] %s", fmt);
  char format[len + 1];
  snprintf(format, len + 1, "[wlr] %s", fmt);  // NOLINT(cert-err33-c)

  zn_vlog_(importance, format, args);
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

  wl_signal_emit(&self->base.events.new_screen, output->screen);
}

static void
zn_default_backend_handle_new_input(struct wl_listener *listener, void *data)
{
  struct zn_default_backend *self =
      zn_container_of(listener, self, new_input_listener);
  struct wlr_input_device *input_device = data;
  struct zn_default_backend_seat *seat =
      zn_default_backend_seat_get(self->base.seat);

  zn_default_backend_seat_handle_new_input(seat, input_device);
}

bool
zn_backend_start(struct zn_backend *base)
{
  struct zn_default_backend *self = zn_default_backend_get(base);

  return wlr_backend_start(self->wlr_backend);
}

struct zn_backend *
zn_backend_create(struct wl_display *display)
{
  wlr_log_init(WLR_DEBUG, handle_wlr_log);
  struct zn_default_backend_seat *seat = NULL;

  struct zn_default_backend *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wl_signal_init(&self->base.events.new_screen);
  self->display = display;

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

  seat = zn_default_backend_seat_create(display, DEFAULT_SEAT);
  if (seat == NULL) {
    zn_error("Failed to create a zn_default_backend_seat");
    goto err_allocator;
  }
  self->base.seat = &seat->base;

  self->new_output_listener.notify = zn_default_backend_handle_new_output;
  wl_signal_add(
      &self->wlr_backend->events.new_output, &self->new_output_listener);

  self->new_input_listener.notify = zn_default_backend_handle_new_input;
  wl_signal_add(
      &self->wlr_backend->events.new_input, &self->new_input_listener);

  return &self->base;

err_allocator:
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

void
zn_backend_destroy(struct zn_backend *base)
{
  struct zn_default_backend *self = zn_default_backend_get(base);
  struct zn_default_backend_seat *seat =
      zn_default_backend_seat_get(self->base.seat);

  wl_list_remove(&self->new_input_listener.link);
  wl_list_remove(&self->new_output_listener.link);
  zn_default_backend_seat_destroy(seat);
  wlr_allocator_destroy(self->wlr_allocator);
  wlr_renderer_destroy(self->wlr_renderer);
  wlr_backend_destroy(self->wlr_backend);
  wl_list_remove(&self->base.events.new_screen.listener_list);
  free(self);
}
