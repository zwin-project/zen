#include "remote.h"

#include <zen-common.h>
#include <zen-remote/server/session.h>

#include "log.h"
#include "loop.h"
#include "peer.h"

struct znr_session *
znr_remote_create_session(
    struct znr_remote *parent, struct znr_remote_peer *peer_base)
{
  znr_remote_impl *self = zn_container_of(parent, self, base);
  znr_remote_peer_impl *peer = zn_container_of(peer_base, peer, base);

  auto loop = wl_display_get_event_loop(self->display);
  auto session_proxy = zen::remote::server::CreateSession(
      std::unique_ptr<zen::remote::ILoop>(new Loop(loop)));

  if (session_proxy->Connect(peer->proxy) == false) {
    zn_warn("Failed to establish session");
    return nullptr;
  };

  auto session = znr_session_create(std::move(session_proxy), self->display);
  if (!session) {
    zn_error("Failed to create a session");
    zn_terminate(EXIT_FAILURE);
    return nullptr;
  };

  return &session->base;
}

static void
znr_remote_handle_new_peer(struct znr_remote_impl *self, uint64_t peer_id)
{
  auto peer_data = self->peer_manager->Get(peer_id);
  if (!peer_data) return;

  auto peer = znr_remote_peer_create(peer_data);
  wl_list_insert(&self->peer_list, &peer->link);
  wl_signal_emit(&self->base.events.new_peer, &peer->base);
}

static void
znr_remote_handle_peer_lost(struct znr_remote_impl *self, uint64_t peer_id)
{
  znr_remote_peer_impl *peer, *tmp;
  wl_list_for_each_safe (peer, tmp, &self->peer_list, link) {
    if (peer->proxy->id() != peer_id) continue;

    wl_list_remove(&peer->link);
    znr_remote_peer_destroy(peer);
    break;
  }
}

struct znr_remote *
znr_remote_create(wl_display *display)
{
  zen::remote::InitializeLogger(std::make_unique<LogSink>());
  wl_event_loop *loop = wl_display_get_event_loop(display);

  auto self = new znr_remote_impl();
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->display = display;
  wl_signal_init(&self->base.events.new_peer);
  wl_list_init(&self->peer_list);

  self->peer_manager = zen::remote::server::CreatePeerManager(
      std::unique_ptr<zen::remote::ILoop>(new Loop(loop)));

  self->peer_manager->on_peer_discover.Connect(
      std::bind(znr_remote_handle_new_peer, self, std::placeholders::_1));
  self->peer_manager->on_peer_lost.Connect(
      std::bind(znr_remote_handle_peer_lost, self, std::placeholders::_1));

  return &self->base;

err:
  return nullptr;
}

void
znr_remote_destroy(znr_remote *parent)
{
  znr_remote_impl *self = zn_container_of(parent, self, base);

  wl_list_remove(&self->base.events.new_peer.listener_list);
  wl_list_remove(&self->peer_list);

  delete self;
}
