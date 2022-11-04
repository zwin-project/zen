#include "zen/remote.h"

#include <zen-common.h>

#include "zen/renderer/system.h"

static void
zn_remote_handle_new_peer(struct wl_listener *listener, void *data)
{
  struct zn_remote *self = zn_container_of(listener, self, new_peer_listener);
  struct znr_remote_peer *peer = data;

  zn_debug("New peer found: %s", peer->host);

  znr_remote_request_new_session(self->renderer, peer);
}

// just for logging
static void
zn_remote_handle_new_session(struct wl_listener *listener, void *data)
{
  UNUSED(listener);
  UNUSED(data);

  zn_debug("New session started");
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

  self->renderer = znr_remote_create(display);
  if (self->renderer == NULL) {
    zn_error("Faied to create znr_remtoe");
    goto err_free;
  }

  self->new_peer_listener.notify = zn_remote_handle_new_peer;
  wl_signal_add(&self->renderer->events.new_peer, &self->new_peer_listener);

  struct znr_system *system = znr_remote_get_system(self->renderer);
  self->new_session_listener.notify = zn_remote_handle_new_session;
  wl_signal_add(&system->events.new_session, &self->new_session_listener);

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
  wl_list_remove(&self->new_session_listener.link);
  znr_remote_destroy(self->renderer);
  free(self);
}
