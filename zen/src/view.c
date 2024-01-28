#include "zen/view.h"

#include <cglm/vec2.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/snode.h"

void
zn_view_set_focus(struct zn_view *self, bool focused)
{
  if (self->has_focus == focused) {
    return;
  }

  self->has_focus = focused;
  self->impl->set_focus(self->impl_data, focused);
}

void
zn_view_configure_size(struct zn_view *self, vec2 size)
{
  self->impl->configure_size(self->impl_data, size);
}

void
zn_view_close(struct zn_view *self)
{
  self->impl->close(self->impl_data);
}

void
zn_view_notify_resized(struct zn_view *self, vec2 size)
{
  struct zn_view_resized_event event;
  glm_vec2_copy(self->size, event.previous_size);
  glm_vec2_copy(size, self->size);
  wl_signal_emit(&self->events.resized, &event);
}

void
zn_view_notify_decoration(
    struct zn_view *self, enum zn_view_decoration_mode mode)
{
  self->decoration_mode = mode;
  wl_signal_emit(&self->events.decoration, NULL);
}

void
zn_view_notify_move_request(struct zn_view *self)
{
  wl_signal_emit(&self->events.move_request, NULL);
}

void
zn_view_notify_resize_request(
    struct zn_view *self, struct zn_view_resize_event *event)
{
  wl_signal_emit(&self->events.resize_request, event);
}

void
zn_view_notify_unmap(struct zn_view *self)
{
  zn_snode_set_position(self->snode, NULL, (vec2){0, 0});

  wl_signal_emit(&self->events.unmap, NULL);
}

struct zn_view *
zn_view_create(void *impl_data, const struct zn_view_interface *implementation)
{
  struct zn_view *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->has_focus = false;

  self->snode = zn_snode_create(self, &zn_snode_noop_implementation);
  if (self->snode == NULL) {
    zn_error("Failed to create a zn_snode");
    goto err_free;
  }

  self->impl_data = impl_data;
  self->impl = implementation;
  self->decoration_mode = ZN_VIEW_DECORATION_MODE_CLIENT_SIDE;

  wl_signal_init(&self->events.resized);
  wl_signal_init(&self->events.unmap);
  wl_signal_init(&self->events.decoration);
  wl_signal_init(&self->events.move_request);
  wl_signal_init(&self->events.resize_request);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_view_destroy(struct zn_view *self)
{
  wl_list_remove(&self->events.resized.listener_list);
  wl_list_remove(&self->events.unmap.listener_list);
  wl_list_remove(&self->events.decoration.listener_list);
  wl_list_remove(&self->events.move_request.listener_list);
  wl_list_remove(&self->events.resize_request.listener_list);
  zn_snode_destroy(self->snode);
  free(self);
}
