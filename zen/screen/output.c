#include "zen/screen/output.h"

#include <pixman.h>
#include <wlr/render/wlr_renderer.h>
#include <zen-common.h>

static void zn_output_destroy(struct zn_output *self);

static void
zn_output_handle_wlr_output_destroy(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_output *self =
      zn_container_of(listener, self, wlr_output_destroy_listener);

  zn_output_destroy(self);
}

static void
zn_output_handle_damage_frame(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_output *self =
      zn_container_of(listener, self, damage_frame_listener);
  struct wlr_renderer *renderer = self->wlr_output->renderer;
  bool needs_frame;
  pixman_region32_t damage;

  pixman_region32_init(&damage);

  if (!wlr_output_damage_attach_render(self->damage, &needs_frame, &damage)) {
    goto damage_finish;
  }

  if (needs_frame) {
    wlr_renderer_begin(
        renderer, self->wlr_output->width, self->wlr_output->height);
    wlr_renderer_clear(renderer, (float[]){1.0f, 1.0f, 0.0f, 1.0f});
    wlr_renderer_end(renderer);
    wlr_output_commit(self->wlr_output);
  } else {
    wlr_output_rollback(self->wlr_output);
  }

damage_finish:
  pixman_region32_fini(&damage);
}

struct zn_output *
zn_output_create(struct wlr_output *wlr_output)
{
  struct zn_output *self;
  struct wlr_output_mode *mode;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    goto err;
  }

  self->wlr_output = wlr_output;
  wlr_output->data = self;

  wlr_output_create_global(wlr_output);

  self->damage = wlr_output_damage_create(wlr_output);
  if (self->damage == NULL) {
    zn_error("Failed to create a wlr_output_damage");
    goto err_free;
  }

  self->wlr_output_destroy_listener.notify =
      zn_output_handle_wlr_output_destroy;
  wl_signal_add(
      &wlr_output->events.destroy, &self->wlr_output_destroy_listener);

  self->damage_frame_listener.notify = zn_output_handle_damage_frame;
  wl_signal_add(&self->damage->events.frame, &self->damage_frame_listener);

  if (wl_list_empty(&self->wlr_output->modes)) {
    zn_error("Failed to get output mode");
    goto err_damage;
  }

  mode = wlr_output_preferred_mode(self->wlr_output);
  if (mode == NULL) {
    zn_error("Failed to get preferred output mode");
    goto err_damage;
  }

  wlr_output_set_mode(self->wlr_output, mode);
  wlr_output_enable(self->wlr_output, true);
  if (wlr_output_commit(self->wlr_output) == false) {
    zn_error("Failed to set output mode");
    goto err_damage;
  }

  return self;

err_damage:
  wlr_output_damage_destroy(self->damage);

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_output_destroy(struct zn_output *self)
{
  wl_list_remove(&self->damage_frame_listener.link);
  wl_list_remove(&self->wlr_output_destroy_listener.link);
  wlr_output_destroy_global(self->wlr_output);
  free(self);
}
