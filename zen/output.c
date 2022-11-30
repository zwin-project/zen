#include "zen/output.h"

#include <math.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_output_damage.h>
#include <wlr/types/wlr_surface.h>

#include "zen-common.h"
#include "zen/input/seat.h"
#include "zen/scene/scene.h"
#include "zen/scene/screen-layout.h"
#include "zen/scene/screen.h"
#include "zen/scene/view.h"
#include "zen/screen-renderer.h"

static void zn_output_destroy(struct zn_output *self);

static inline int
scale_length(float length, float offset, float scale)
{
  return round((offset + length) * scale) - round(offset * scale);
}

void
zn_output_box_effective_to_transformed_coords(struct zn_output *self,
    struct wlr_fbox *effective, struct wlr_box *transformed)
{
  float scale = self->wlr_output->scale;
  transformed->x = round(effective->x * scale);
  transformed->y = round(effective->y * scale);
  transformed->width = scale_length(effective->width, effective->x, scale);
  transformed->height = scale_length(effective->height, effective->y, scale);
}

static void
zn_output_handle_wlr_output_destroy(struct wl_listener *listener, void *data)
{
  struct zn_output *self =
      zn_container_of(listener, self, wlr_output_destroy_listener);
  UNUSED(data);

  zn_output_destroy(self);
}

void
zn_output_add_damage_box(struct zn_output *self, struct wlr_fbox *effective_box)
{
  struct wlr_box box;
  zn_output_box_effective_to_transformed_coords(self, effective_box, &box);
  wlr_output_damage_add_box(self->damage, &box);
}

void
zn_output_add_damage_whole(struct zn_output *self)
{
  wlr_output_damage_add_whole(self->damage);
}

static int
zn_output_repaint_timer_handler(void *data)
{
  struct zn_output *self = data;
  struct wlr_renderer *renderer = self->server->renderer;
  pixman_region32_t damage;
  bool needs_frame;

  self->wlr_output->frame_pending = false;

  pixman_region32_init(&damage);

  if (!wlr_output_damage_attach_render(self->damage, &needs_frame, &damage)) {
    goto damage_finish;
  }

  if (needs_frame) {
    zn_screen_renderer_render(self->screen, renderer, &damage);
  } else {
    wlr_output_rollback(self->wlr_output);
  }

damage_finish:
  pixman_region32_fini(&damage);
  return 0;
}

static void
send_frame_done_callback(struct wlr_surface *surface, void *user_data)
{
  wlr_surface_send_frame_done(surface, user_data);
}

static void
zn_output_handle_damage_frame(struct wl_listener *listener, void *data)
{
  struct zn_output *self =
      zn_container_of(listener, self, damage_frame_listener);
  struct zn_server *server = zn_server_get_singleton();
  struct timespec now;
  UNUSED(data);

  if (self->wlr_output->enabled == false) return;

  if (server->display_system == ZEN_DISPLAY_SYSTEM_TYPE_IMMERSIVE) {
    // TODO: Display an indication that an immersive display system is in use.
    return;
  }

  // TODO: add delay to call zn_output_repaint_timer_handler

  zn_output_repaint_timer_handler(self);

  clock_gettime(CLOCK_MONOTONIC, &now);
  zn_screen_for_each_visible_surface(
      self->screen, send_frame_done_callback, &now);
}

struct zn_output *
zn_output_create(struct wlr_output *wlr_output, struct zn_server *server)
{
  struct zn_output *self;
  struct wl_event_loop *loop = server->loop;
  struct wlr_output_mode *mode;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wlr_output->data = self;
  self->wlr_output = wlr_output;
  self->server = server;

  wlr_output_create_global(wlr_output);

  self->damage = wlr_output_damage_create(self->wlr_output);
  if (self->damage == NULL) {
    zn_error("Failed to create wlr_output_damage");
    goto err_free;
  }

  self->wlr_output_destroy_listener.notify =
      zn_output_handle_wlr_output_destroy;
  wl_signal_add(
      &self->wlr_output->events.destroy, &self->wlr_output_destroy_listener);

  self->damage_frame_listener.notify = zn_output_handle_damage_frame;
  wl_signal_add(&self->damage->events.frame, &self->damage_frame_listener);

  self->repaint_timer =
      wl_event_loop_add_timer(loop, zn_output_repaint_timer_handler, self);

  if (wl_list_empty(&self->wlr_output->modes)) {
    zn_warn("Failed to get output mode");
    goto err_damage;
  }

  mode = wlr_output_preferred_mode(self->wlr_output);
  if (mode == NULL) {
    zn_warn("Failed to get preferred output mode");
    goto err_repaint_timer;
  }

  wlr_output_set_mode(self->wlr_output, mode);
  wlr_output_enable(self->wlr_output, true);
  if (wlr_output_commit(self->wlr_output) == false) {
    zn_warn("Failed to set output mode");
    goto err_repaint_timer;
  }

  // zn_screen must create after configuration of wlr_output
  self->screen = zn_screen_create(server->scene->screen_layout, self);
  if (self->screen == NULL) {
    zn_error("Failed to create zn_screen");
    goto err_repaint_timer;
  }

  return self;

err_repaint_timer:
  wl_event_source_remove(self->repaint_timer);
  wl_list_remove(&self->wlr_output_destroy_listener.link);
  wl_list_remove(&self->damage_frame_listener.link);
  zn_screen_destroy(self->screen);

err_damage:
  wlr_output_damage_destroy(self->damage);

err_free:
  wlr_output_destroy_global(self->wlr_output);
  free(self);

err:
  return NULL;
}

/* only zn_output_wlr_output_destroy_handler should call this */
static void
zn_output_destroy(struct zn_output *self)
{
  wl_event_source_remove(self->repaint_timer);
  zn_screen_destroy(self->screen);
  wlr_output_destroy_global(self->wlr_output);
  free(self);
}
