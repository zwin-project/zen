#include "zen/peer.h"

#include <zen-common.h>

#include "zen/server.h"

static void zn_peer_destroy(struct zn_peer *self);

static void
zn_peer_handle_znr_remote_peer_destroy(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_peer *self =
      zn_container_of(listener, self, znr_remote_peer_destroy_listener);

  zn_peer_destroy(self);
}

struct zn_peer *
zn_peer_create(struct znr_remote_peer *znr_remote_peer)
{
  struct zn_peer *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->znr_remote_peer_destroy_listener.notify =
      zn_peer_handle_znr_remote_peer_destroy;
  wl_signal_add(&znr_remote_peer->events.destroy,
      &self->znr_remote_peer_destroy_listener);

  self->znr_remote_peer = znr_remote_peer;

  wl_list_init(&self->link);

  return self;

err:
  return NULL;
}

static void
zn_peer_destroy(struct zn_peer *self)
{
  struct zn_server *server = zn_server_get_singleton();

  wl_list_remove(&self->link);
  wl_list_remove(&self->znr_remote_peer_destroy_listener.link);
  free(self);

  wl_signal_emit(&server->remote->events.peer_list_changed, NULL);
}
