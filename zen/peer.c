#include "zen/peer.h"

#include <string.h>
#include <zen-common.h>

#include "zen/renderer/session.h"
#include "zen/server.h"

static void zn_peer_destroy(struct zn_peer *self);

static void
zn_peer_handle_znr_remote_peer_destroy(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_peer *self =
      zn_container_of(listener, self, znr_remote_peer_destroy_listener);

  self->znr_remote_peer = NULL;

  if (!self->session) {
    zn_peer_destroy(self);
  }
}

static void
zn_peer_handle_znr_session_disconnected(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_peer *self =
      zn_container_of(listener, self, znr_session_disconnected_listener);
  zn_peer_destroy(self);
}

void
zn_peer_set_session(struct zn_peer *self, struct znr_session *session)
{
  if (!zn_assert(!self->session, "session already exists")) {
    return;
  }

  self->session = session;
  wl_signal_add(
      &session->events.disconnected, &self->znr_session_disconnected_listener);
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

  self->host = strdup(znr_remote_peer->host);
  if (!self->host) {
    zn_error("Failed to duplicate host name");
    goto err_free;
  }

  self->znr_remote_peer_destroy_listener.notify =
      zn_peer_handle_znr_remote_peer_destroy;
  wl_signal_add(&znr_remote_peer->events.destroy,
      &self->znr_remote_peer_destroy_listener);

  self->znr_session_disconnected_listener.notify =
      zn_peer_handle_znr_session_disconnected;
  wl_list_init(&self->znr_session_disconnected_listener.link);

  self->znr_remote_peer = znr_remote_peer;

  wl_list_init(&self->link);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_peer_destroy(struct zn_peer *self)
{
  struct zn_server *server = zn_server_get_singleton();

  free(self->host);
  wl_list_remove(&self->znr_session_disconnected_listener.link);
  wl_list_remove(&self->link);
  wl_list_remove(&self->znr_remote_peer_destroy_listener.link);
  free(self);

  wl_signal_emit(&server->remote->events.peer_list_changed, NULL);
}
