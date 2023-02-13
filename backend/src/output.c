#include "zen/backend/output.h"

#include <wayland-server-core.h>
#include <wayland-util.h>

#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"
#include "zen/screen.h"

static void zn_output_destroy(struct zn_output *self);

static void
zn_output_handle_damage_frame(struct wl_listener *listener, void *data UNUSED)
{
  struct zn_output *self =
      zn_container_of(listener, self, damage_frame_listener);

  struct zn_screen_frame_event event;

  // TODO(@Aki-7): fill event

  zn_screen_notify_frame(self->screen, &event);
}

static void
zn_output_handle_wlr_output_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_output *self =
      zn_container_of(listener, self, wlr_output_destroy_listener);

  zn_output_destroy(self);
}

struct zn_output *
zn_output_create(struct wlr_output *wlr_output)
{
  struct zn_output *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->wlr_output = wlr_output;

  self->screen = zn_screen_create(self);
  if (self->screen == NULL) {
    zn_error("Failed to create a zn_screen");
    goto err_free;
  }

  self->damage = wlr_output_damage_create(wlr_output);
  if (self->damage == NULL) {
    zn_error("Failed to create a wlr_output_damage");
    goto err_screen;
  }

  struct wlr_output_mode *mode = wlr_output_preferred_mode(self->wlr_output);
  if (mode == NULL) {
    zn_error("Output %p doesn't support modes", (void *)wlr_output);
    goto err_damage;
  }

  wlr_output_set_mode(self->wlr_output, mode);
  wlr_output_enable(self->wlr_output, true);
  if (!wlr_output_commit(self->wlr_output)) {
    zn_error("Failed to commit initial status");
    goto err_damage;
  }

  self->wlr_output_destroy_listener.notify =
      zn_output_handle_wlr_output_destroy;
  wl_signal_add(
      &self->wlr_output->events.destroy, &self->wlr_output_destroy_listener);

  self->damage_frame_listener.notify = zn_output_handle_damage_frame;
  wl_signal_add(&self->damage->events.frame, &self->damage_frame_listener);

  return self;

err_damage:
  wlr_output_damage_destroy(self->damage);

err_screen:
  zn_screen_destroy(self->screen);

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_output_destroy(struct zn_output *self)
{
  zn_screen_destroy(self->screen);
  wl_list_remove(&self->wlr_output_destroy_listener.link);
  free(self);
}
