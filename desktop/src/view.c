#include "zen-desktop/view.h"

#include <cglm/vec2.h>

#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"
#include "zen-desktop/cursor-grab/move.h"
#include "zen-desktop/ui/decoration.h"
#include "zen/snode.h"
#include "zen/view.h"

static void zn_desktop_view_destroy(struct zn_desktop_view *self);

static void
zn_desktop_view_handle_zn_view_resized(
    struct wl_listener *listener, void *user_data UNUSED)
{
  struct zn_desktop_view *self =
      zn_container_of(listener, self, zn_view_resized_listener);

  zn_ui_decoration_set_content_size(self->decoration, self->zn_view->size);
  zn_snode_set_position(
      self->zn_view->snode, self->snode, self->decoration->content_offset);
}

static void
zn_desktop_view_handle_zn_view_move(
    struct wl_listener *listener, void *user_data UNUSED)
{
  struct zn_desktop_view *self =
      zn_container_of(listener, self, zn_view_move_listener);
  zn_cursor_move_grab_start(self);
}

static void
zn_desktop_view_handle_zn_view_unmap(
    struct wl_listener *listener, void *user_data UNUSED)
{
  struct zn_desktop_view *self =
      zn_container_of(listener, self, zn_view_unmap_listener);

  zn_desktop_view_destroy(self);
}

struct zn_desktop_view *
zn_desktop_view_create(struct zn_view *zn_view)
{
  struct zn_desktop_view *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zn_view = zn_view;
  wl_signal_init(&self->events.destroy);

  self->snode = zn_snode_create(self, &zn_snode_noop_implementation);
  if (self->snode == NULL) {
    zn_error("Failed to create a zn_snode");
    goto err_free;
  }

  self->decoration = zn_ui_decoration_create();
  if (self->decoration == NULL) {
    zn_error("Failed to create a zn_ui_decoration");
    goto err_snode;
  }

  self->zn_view_resized_listener.notify =
      zn_desktop_view_handle_zn_view_resized;
  wl_signal_add(&zn_view->events.resized, &self->zn_view_resized_listener);

  self->zn_view_unmap_listener.notify = zn_desktop_view_handle_zn_view_unmap;
  wl_signal_add(&zn_view->events.unmap, &self->zn_view_unmap_listener);

  self->zn_view_move_listener.notify = zn_desktop_view_handle_zn_view_move;
  wl_signal_add(&zn_view->events.move, &self->zn_view_move_listener);

  zn_snode_set_position(self->decoration->snode, self->snode, GLM_VEC2_ZERO);

  zn_ui_decoration_set_content_size(self->decoration, zn_view->size);
  zn_snode_set_position(
      zn_view->snode, self->snode, self->decoration->content_offset);

  return self;

err_snode:
  zn_snode_destroy(self->snode);

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_desktop_view_destroy(struct zn_desktop_view *self)
{
  zn_signal_emit_mutable(&self->events.destroy, NULL);

  zn_ui_decoration_destroy(self->decoration);
  zn_snode_destroy(self->snode);
  wl_list_remove(&self->zn_view_move_listener.link);
  wl_list_remove(&self->zn_view_unmap_listener.link);
  wl_list_remove(&self->zn_view_resized_listener.link);
  wl_list_remove(&self->events.destroy.listener_list);
  free(self);
}
