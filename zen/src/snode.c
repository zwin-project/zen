#include "zen/snode.h"

#include <cglm/vec2.h>

#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"
#include "zen/screen.h"

static struct wlr_texture *
get_texture(void *user_data UNUSED)
{
  return NULL;
}

static const struct zn_snode_interface noop = {
    .get_texture = get_texture,
};

static void
zn_snode_damage_whole(struct zn_snode *self)
{
  struct wlr_fbox box;
  if (!self->screen) {
    return;
  }

  zn_snode_get_fbox(self, &box);
  zn_screen_damage(self->screen, &box);
}

static void
zn_snode_cache_absolute_position(struct zn_snode *self)
{
  if (self->parent) {
    glm_vec2_add(self->parent->cached_absolute_position, self->position,
        self->cached_absolute_position);
  } else {
    glm_vec2_copy(self->position, self->cached_absolute_position);
  }
}

void
zn_snode_set_position(
    struct zn_snode *self, struct zn_snode *parent, float x, float y)
{
  zn_snode_damage_whole(self);

  if (self->parent) {
    wl_list_remove(&self->link);
    wl_list_init(&self->link);
    wl_list_remove(&self->parent_destroy_listener.link);
    wl_list_init(&self->parent_destroy_listener.link);
    wl_list_remove(&self->parent_position_changed_listener.link);
    wl_list_init(&self->parent_position_changed_listener.link);
  }

  if (parent) {
    wl_list_insert(&parent->child_node_list, &self->link);
    wl_signal_add(&parent->events.destroy, &self->parent_destroy_listener);
    wl_signal_add(&parent->events.position_changed,
        &self->parent_position_changed_listener);
  }

  self->parent = parent;
  self->screen = parent ? parent->screen : NULL;
  self->position[0] = x;
  self->position[1] = y;

  zn_snode_cache_absolute_position(self);

  zn_snode_damage_whole(self);

  wl_signal_emit(&self->events.position_changed, NULL);
}

struct wlr_texture *
zn_snode_get_texture(struct zn_snode *self)
{
  return self->impl->get_texture(self->user_data);
}

void
zn_snode_get_fbox(struct zn_snode *self, struct wlr_fbox *fbox)
{
  struct wlr_texture *texture = self->impl->get_texture(self->user_data);
  fbox->x = self->cached_absolute_position[0];
  fbox->y = self->cached_absolute_position[1];
  if (texture) {
    fbox->width = texture->width;
    fbox->height = texture->height;
  } else {
    fbox->width = 0;
    fbox->height = 0;
  }
}

static void
zn_snode_handle_parent_position_changed(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_snode *self =
      zn_container_of(listener, self, parent_position_changed_listener);

  zn_snode_damage_whole(self);

  zn_snode_cache_absolute_position(self);
  self->screen = self->parent->screen;

  zn_snode_damage_whole(self);

  wl_signal_emit(&self->events.position_changed, NULL);
}

static void
zn_snode_handle_parent_destroy(struct wl_listener *listener, void *data UNUSED)
{
  struct zn_snode *self =
      zn_container_of(listener, self, parent_destroy_listener);

  zn_snode_set_position(self, NULL, 0, 0);
}

struct zn_snode *
zn_snode_create(
    void *user_data, const struct zn_snode_interface *implementation)
{
  struct zn_snode *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->user_data = user_data;
  self->impl = implementation;

  self->parent = NULL;
  glm_vec2_zero(self->position);
  glm_vec2_zero(self->cached_absolute_position);

  self->screen = NULL;

  wl_list_init(&self->child_node_list);
  wl_list_init(&self->link);

  self->parent_destroy_listener.notify = zn_snode_handle_parent_destroy;
  wl_list_init(&self->parent_destroy_listener.link);

  self->parent_position_changed_listener.notify =
      zn_snode_handle_parent_position_changed;
  wl_list_init(&self->parent_position_changed_listener.link);

  wl_signal_init(&self->events.position_changed);
  wl_signal_init(&self->events.destroy);

  return self;

err:
  return NULL;
}

struct zn_snode *
zn_snode_create_root(struct zn_screen *screen)
{
  struct zn_snode *self = zn_snode_create(NULL, &noop);
  self->screen = screen;
  return self;
}

void
zn_snode_destroy(struct zn_snode *self)
{
  zn_signal_emit_mutable(&self->events.destroy, NULL);

  wl_list_remove(&self->events.destroy.listener_list);
  wl_list_remove(&self->events.position_changed.listener_list);
  wl_list_remove(&self->parent_position_changed_listener.link);
  wl_list_remove(&self->parent_destroy_listener.link);
  wl_list_remove(&self->link);
  wl_list_remove(&self->child_node_list);
  free(self);
}
