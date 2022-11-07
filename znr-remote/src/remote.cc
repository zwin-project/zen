#include "remote.h"

#include <zen-common.h>
#include <zen-remote/server/session.h>

#include "log.h"
#include "loop.h"
#include "peer.h"

void
znr_remote_request_new_session(znr_remote* parent, znr_remote_peer* peer_base)
{
  znr_remote_impl* self = zn_container_of(parent, self, base);
  znr_remote_peer_impl* peer = zn_container_of(peer_base, peer, base);

  auto loop = wl_display_get_event_loop(self->display);
  auto session_proxy = zen::remote::server::CreateSession(
      std::unique_ptr<zen::remote::ILoop>(new Loop(loop)));

  if (session_proxy->Connect(peer->proxy) == false) return;

  auto session = znr_session_create(std::move(session_proxy));
  if (!session) return;

  if (self->system.current_session) {
    znr_session_impl* current =
        zn_container_of(self->system.current_session, current, base);
    znr_session_destroy(current);
    self->system.current_session = NULL;
    wl_signal_emit(&self->system.events.current_session_destroyed, NULL);
  }

  self->system.current_session = &session->base;
  wl_signal_emit(&self->system.events.current_session_created, NULL);
}

struct znr_system*
znr_remote_get_system(struct znr_remote* parent)
{
  znr_remote_impl* self = zn_container_of(parent, self, base);

  return &self->system;
}

static void
znr_remote_handle_new_peer(struct znr_remote_impl* self, uint64_t peer_id)
{
  auto peer_data = self->peer_manager->Get(peer_id);
  if (!peer_data) return;

  auto peer = znr_remote_peer_create(peer_data);
  wl_list_insert(&self->peer_list, &peer->link);
  wl_signal_emit(&self->base.events.new_peer, &peer->base);
}

static void
znr_remote_handle_peer_lost(struct znr_remote_impl* self, uint64_t peer_id)
{
  znr_remote_peer_impl *peer, *tmp;
  wl_list_for_each_safe (peer, tmp, &self->peer_list, link) {
    if (peer->proxy->id() != peer_id) continue;

    wl_list_remove(&peer->link);
    znr_remote_peer_destroy(peer);
    break;
  }
}

struct znr_remote*
znr_remote_create(wl_display* display)
{
  zen::remote::InitializeLogger(std::make_unique<LogSink>());
  wl_event_loop* loop = wl_display_get_event_loop(display);

  auto self = new znr_remote_impl();
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->display = display;
  self->system.current_session = nullptr;
  wl_signal_init(&self->base.events.new_peer);
  wl_signal_init(&self->system.events.current_session_destroyed);
  wl_signal_init(&self->system.events.current_session_created);
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
znr_remote_destroy(znr_remote* parent)
{
  znr_remote_impl* self = zn_container_of(parent, self, base);

  if (self->system.current_session) {
    struct znr_session_impl* session =
        zn_container_of(self->system.current_session, session, base);
    znr_session_destroy(session);
  }
  wl_list_remove(&self->system.events.current_session_created.listener_list);
  wl_list_remove(&self->system.events.current_session_destroyed.listener_list);
  wl_list_remove(&self->base.events.new_peer.listener_list);
  wl_list_remove(&self->peer_list);

  delete self;
}
