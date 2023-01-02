#include "zen/remote.h"

#include <zen-common.h>

#include "zen/peer.h"
#include "zen/server.h"

static void
zn_remote_handle_new_peer(struct wl_listener *listener, void *data)
{
  struct zn_remote *self = zn_container_of(listener, self, new_peer_listener);
  struct znr_remote_peer *znr_remote_peer = data;

  struct zn_peer *peer = zn_peer_create(znr_remote_peer);
  if (peer == NULL) {
    zn_error("Failed to create a zn_peer");
    return;
  }

  wl_list_insert(&self->peer_list, &peer->link);

  wl_signal_emit(&self->events.peer_list_changed, NULL);
}

struct zn_remote *
zn_remote_create(struct wl_display *display)
{
  struct zn_remote *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->znr_remote = znr_remote_create(display);
  if (self->znr_remote == NULL) {
    zn_error("Failed to create a znr_remote");
    goto err_free;
  }

  self->new_peer_listener.notify = zn_remote_handle_new_peer;
  wl_signal_add(&self->znr_remote->events.new_peer, &self->new_peer_listener);

  wl_list_init(&self->peer_list);
  wl_signal_init(&self->events.peer_list_changed);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_remote_destroy(struct zn_remote *self)
{
  wl_list_remove(&self->new_peer_listener.link);
  wl_list_remove(&self->peer_list);
  wl_list_remove(&self->events.peer_list_changed.listener_list);
  znr_remote_destroy(self->znr_remote);
  free(self);
}
