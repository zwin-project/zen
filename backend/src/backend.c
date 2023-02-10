#include "zen/backend/backend.h"

#include <stdio.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_output.h>
#include <wlr/util/log.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/backend/output.h"

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

  struct zn_output *output = zn_output_create(wlr_output);

  wl_signal_emit(&self->base.events.new_screen, output->screen);
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

  self->new_output_listener.notify = zn_default_backend_handle_new_output;
  wl_signal_add(
      &self->wlr_backend->events.new_output, &self->new_output_listener);

  return &self->base;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_backend_destroy(struct zn_backend *base)
{
  struct zn_default_backend *self = zn_default_backend_get(base);

  wl_list_remove(&self->new_output_listener.link);
  wlr_backend_destroy(self->wlr_backend);
  wl_list_remove(&self->base.events.new_screen.listener_list);
  free(self);
}
