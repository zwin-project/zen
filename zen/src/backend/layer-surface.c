#include "layer-surface.h"

#include <zen/snode.h>

#include "output.h"
#include "screen-backend.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/screen.h"

static void zn_layer_surface_destroy(struct zn_layer_surface *self);

static void
zn_layer_surface_handle_surface_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_layer_surface *self =
      zn_container_of(listener, self, surface_destroy_listener);

  zn_layer_surface_destroy(self);
}

static void
zn_layer_surface_handle_surface_map(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_layer_surface *self =
      zn_container_of(listener, self, surface_map_listener);

  wl_signal_add(&self->wlr_layer_surface->surface->events.commit,
      &self->surface_commit_listener);

  zn_snode_damage_whole(self->snode);
}

static void
zn_layer_surface_handle_surface_unmap(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_layer_surface *self =
      zn_container_of(listener, self, surface_unmap_listener);

  wl_list_remove(&self->surface_commit_listener.link);
  wl_list_init(&self->surface_commit_listener.link);

  zn_snode_damage_whole(self->snode);
}

static void
zn_layer_surface_handle_surface_commit(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_layer_surface *self =
      zn_container_of(listener, self, surface_commit_listener);

  bool layer_changed = self->layer != self->wlr_layer_surface->current.layer;

  if (layer_changed) {
    zn_screen_backend_add_layer_surface(self->output->screen_backend, self,
        self->wlr_layer_surface->current.layer);
    zn_snode_damage_whole(self->snode);
  } else {
    pixman_region32_t damage;

    pixman_region32_init(&damage);

    wlr_surface_get_effective_damage(self->wlr_layer_surface->surface, &damage);

    pixman_box32_t *rects = NULL;
    int rect_count = 0;

    rects = pixman_region32_rectangles(&damage, &rect_count);

    for (int i = 0; i < rect_count; i++) {
      struct wlr_fbox damage_fbox = {
          .x = rects[i].x1,
          .y = rects[i].y1,
          .width = rects[i].x2 - rects[i].x1,
          .height = rects[i].y2 - rects[i].y1,
      };

      zn_snode_damage(self->snode, &damage_fbox);
    }

    pixman_region32_fini(&damage);
  }
}

static void
zn_layer_surface_handle_output_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_layer_surface *self =
      zn_container_of(listener, self, output_destroy_listener);

  // This triggers surface_destroy_listener and then destroys self.
  wlr_layer_surface_v1_destroy(self->wlr_layer_surface);
}

struct wlr_texture *
zn_layer_surface_get_texture(void *user_data)
{
  struct zn_layer_surface *self = user_data;
  struct wlr_texture *texture =
      wlr_surface_get_texture(self->wlr_layer_surface->surface);

  return texture;
}

void
zn_layout_surface_frame(void *user_data, const struct timespec *when)
{
  struct zn_layer_surface *self = user_data;

  wlr_surface_send_frame_done(self->wlr_layer_surface->surface, when);
}

const struct zn_snode_interface zn_layer_surface_snode_implementation = {
    .get_texture = zn_layer_surface_get_texture,
    .frame = zn_snode_noop_frame,
    .accepts_input = zn_snode_noop_accepts_input,
    .pointer_button = zn_snode_noop_pointer_button,
    .pointer_enter = zn_snode_noop_pointer_enter,
    .pointer_motion = zn_snode_noop_pointer_motion,
    .pointer_leave = zn_snode_noop_pointer_leave,
    .pointer_axis = zn_snode_noop_pointer_axis,
    .pointer_frame = zn_snode_noop_pointer_frame,
};

struct zn_layer_surface *
zn_layer_surface_create(
    struct wlr_layer_surface_v1 *wlr_layer_surface, struct zn_output *output)
{
  struct zn_layer_surface *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->output = output;
  self->wlr_layer_surface = wlr_layer_surface;
  self->layer = wlr_layer_surface->current.layer;

  self->snode = zn_snode_create(self, &zn_layer_surface_snode_implementation);
  if (self->snode == NULL) {
    zn_error("Failed to create a zn_snode");
    goto err_free;
  }

  self->surface_destroy_listener.notify =
      zn_layer_surface_handle_surface_destroy;
  wl_signal_add(
      &wlr_layer_surface->events.destroy, &self->surface_destroy_listener);

  self->surface_map_listener.notify = zn_layer_surface_handle_surface_map;
  wl_signal_add(&wlr_layer_surface->events.map, &self->surface_map_listener);

  self->surface_unmap_listener.notify = zn_layer_surface_handle_surface_unmap;
  wl_signal_add(
      &wlr_layer_surface->events.unmap, &self->surface_unmap_listener);

  self->surface_commit_listener.notify = zn_layer_surface_handle_surface_commit;
  wl_list_init(&self->surface_commit_listener.link);

  self->output_destroy_listener.notify = zn_layer_surface_handle_output_destroy;
  wl_signal_add(&output->events.destroy, &self->output_destroy_listener);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_layer_surface_destroy(struct zn_layer_surface *self)
{
  wl_list_remove(&self->output_destroy_listener.link);
  wl_list_remove(&self->surface_commit_listener.link);
  wl_list_remove(&self->surface_unmap_listener.link);
  wl_list_remove(&self->surface_map_listener.link);
  wl_list_remove(&self->surface_destroy_listener.link);
  zn_snode_destroy(self->snode);
  free(self);
}
