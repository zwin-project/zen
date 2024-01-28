#include "xr-system-manager.hh"

#include "log.hh"
#include "loop.hh"
#include "xr-system.hh"
#include "zen-common/log.h"

static void zn_xr_system_manager_destroy_remote(
    struct zn_xr_system_manager *c_obj);

namespace zen::backend::immersive::remote {

XrSystemManager::XrSystemManager(wl_display *display) : display_(display) {}

const zn_xr_system_manager_interface XrSystemManager::c_implementation_ = {
    XrSystemManager::HandleDestroy,
};

bool
XrSystemManager::Init()
{
  zen::remote::InitializeLogger(std::make_unique<LogSink>());

  auto loop = std::make_unique<Loop>(wl_display_get_event_loop(display_));

  peer_manager_ = zen::remote::server::CreatePeerManager(std::move(loop));
  if (!peer_manager_) {
    zn_error("Failed to create a peer manager");
    return false;
  }

  c_obj_.impl_data = this;
  c_obj_.impl = &c_implementation_;
  wl_signal_init(&c_obj_.events.new_system);

  peer_manager_->on_peer_discover.Connect(
      [this](uint64_t id) { HandleRemotePeerDiscover(id); });

  peer_manager_->on_peer_lost.Connect(
      [this](uint64_t id) { HandleRemotePeerLost(id); });

  return true;
}

void
XrSystemManager::HandleDestroy(zn_xr_system_manager *c_obj)
{
  zn_xr_system_manager_destroy_remote(c_obj);
}

void
XrSystemManager::RemoveDeadXrSystem()
{
  auto result = std::remove_if(xr_systems_.begin(), xr_systems_.end(),
      [](std::unique_ptr<XrSystem> &xr_system) {
        return !xr_system->is_alive();
      });

  xr_systems_.erase(result, xr_systems_.end());
}

void
XrSystemManager::HandleRemotePeerDiscover(uint64_t peer_id)
{
  std::for_each(xr_systems_.begin(), xr_systems_.end(),
      [peer_id](std::unique_ptr<XrSystem> &xr_system) {
        if (xr_system->peer_id() == peer_id) {
          xr_system->Kill();
        }
      });
  RemoveDeadXrSystem();

  auto peer = peer_manager_->Get(peer_id);

  zn_debug(
      "Discover a new remote peer %ld (%s)", peer_id, peer->host().c_str());

  auto xr_system = XrSystem::New(peer, display_, this);

  auto *xr_system_c_obj = xr_system->c_obj();

  xr_systems_.push_back(std::move(xr_system));

  wl_signal_emit(&c_obj_.events.new_system, xr_system_c_obj);
}

void
XrSystemManager::HandleRemotePeerLost(uint64_t peer_id)
{
  std::for_each(xr_systems_.begin(), xr_systems_.end(),
      [peer_id](std::unique_ptr<XrSystem> &xr_system) {
        if (xr_system->peer_id() == peer_id && !xr_system->is_connected()) {
          xr_system->Kill();
        }
      });
  RemoveDeadXrSystem();

  zn_debug("Lost a remote peer %ld", peer_id);
}

}  // namespace zen::backend::immersive::remote

struct zn_xr_system_manager *
zn_xr_system_manager_create_remote(struct wl_display *display)
{
  using XrSystemManager = zen::backend::immersive::remote::XrSystemManager;

  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
  auto *self = new XrSystemManager(display);

  if (!self->Init()) {
    zn_error("Failed to initialize remote XrSystemManager");
    delete self;  // NOLINT(cppcoreguidelines-owning-memory)
    return nullptr;
  }

  return &self->c_obj_;
}

static void
zn_xr_system_manager_destroy_remote(struct zn_xr_system_manager *c_obj)
{
  using XrSystemManager = zen::backend::immersive::remote::XrSystemManager;

  auto *self = static_cast<XrSystemManager *>(c_obj->impl_data);

  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
  delete self;
}
