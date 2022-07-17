#include "output.h"

#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_output_damage.h>

#include "zen-common.h"
#include "zen-scene.h"

struct zn_output {
  struct wlr_output *wlr_output;  // nonnull
  struct zn_server *server;       // nonnull

  /** nonnull, automatically destroyed when wlr_output is destroyed */
  struct wlr_output_damage *damage;

  struct zn_scene_output *scene_output;  // controlled by zn_output

  // TODO: use this for better repaint loop
  struct wl_event_source *repaint_timer;

  struct wl_listener wlr_output_destroy_listener;
  struct wl_listener damage_frame_listener;
};

static void zn_output_destroy(struct zn_output *self);

static void
zn_output_wlr_output_destroy_handler(struct wl_listener *listener, void *data)
{
  struct zn_output *self =
      zn_container_of(listener, self, wlr_output_destroy_listener);
  UNUSED(data);

  zn_output_destroy(self);
}

static int
zn_output_repaint_timer_handler(void *data)
{
  struct zn_output *self = data;
  struct wlr_renderer *renderer = zn_server_get_renderer(self->server);

  self->wlr_output->frame_pending = false;

  // FIXME:
  {
    struct timespec now;
    int64_t now_nsec;
    float red;
    pixman_region32_t damage;
    bool needs_frame;
    pixman_region32_init(&damage);

    wlr_output_damage_attach_render(self->damage, &needs_frame, &damage);

    wlr_renderer_begin(
        renderer, self->wlr_output->width, self->wlr_output->height);

    clock_gettime(CLOCK_MONOTONIC, &now);
    now_nsec = timespec_to_nsec(&now);
    red = (float)(now_nsec % (NSEC_PER_SEC * 2)) / (float)NSEC_PER_SEC - 1.0;
    red = red < 0 ? -red : red;

    wlr_renderer_clear(renderer, (float[]){red, 1, 0, 1});

    wlr_renderer_end(renderer);
    pixman_region32_fini(&damage);

    wlr_output_commit(self->wlr_output);
  }

  return 0;
}

static void
zn_output_damage_frame_handler(struct wl_listener *listener, void *data)
{
  struct zn_output *self =
      zn_container_of(listener, self, damage_frame_listener);
  UNUSED(data);

  if (self->wlr_output->enabled == false) return;

  // TODO: add delay to call zn_output_repaint_timer_handler

  zn_output_repaint_timer_handler(self);

  // TODO: Send frame done to all visible surfaces
}

struct zn_output *
zn_output_create(struct wlr_output *wlr_output, struct zn_server *server)
{
  struct zn_output *self;
  struct wl_event_loop *loop = zn_server_get_loop(server);
  struct wlr_output_mode *mode;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wlr_output->data = self;
  self->wlr_output = wlr_output;
  self->server = server;

  self->damage = wlr_output_damage_create(self->wlr_output);
  if (self->damage == NULL) {
    zn_error("Failed to create wlr_output_damage");
    goto err_free;
  }

  self->scene_output =
      zn_scene_output_create(zn_server_get_scene(server), wlr_output);
  if (self->scene_output == NULL) {
    zn_error("Failed to create zn_scene_output");
    goto err_damage;
  }

  self->wlr_output_destroy_listener.notify =
      zn_output_wlr_output_destroy_handler;
  wl_signal_add(
      &self->wlr_output->events.destroy, &self->wlr_output_destroy_listener);

  self->damage_frame_listener.notify = zn_output_damage_frame_handler;
  wl_signal_add(&self->damage->events.frame, &self->damage_frame_listener);

  self->repaint_timer =
      wl_event_loop_add_timer(loop, zn_output_repaint_timer_handler, self);

  if (wl_list_empty(&self->wlr_output->modes)) {
    zn_warn("Failed to get output mode");
    goto err_repaint_timer;
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

  return self;

err_repaint_timer:
  wl_event_source_remove(self->repaint_timer);
  wl_list_remove(&self->wlr_output_destroy_listener.link);
  wl_list_remove(&self->damage_frame_listener.link);
  zn_scene_output_destroy(self->scene_output);

err_damage:
  wlr_output_damage_destroy(self->damage);

err_free:
  free(self);

err:
  return NULL;
}

/* only zn_output_wlr_output_destroy_handler should call this */
static void
zn_output_destroy(struct zn_output *self)
{
  wl_event_source_remove(self->repaint_timer);
  zn_scene_output_destroy(self->scene_output);
  free(self);
}
