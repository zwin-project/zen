#include "xr.h"

#include "loop.h"
#include "remote-system.h"
#include "zen-common/log.h"

namespace zen::xr {

std::unique_ptr<Xr>
CreateXr(wl_display *display, XrDelegate *delegate)
{
  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
  auto *xr = new XrImpl(display, delegate);

  if (!xr->Init()) {
    delete xr;  // NOLINT(cppcoreguidelines-owning-memory)
    return nullptr;
  }

  return std::unique_ptr<Xr>(xr);
}

XrImpl::XrImpl(wl_display *display, XrDelegate *delegate)
    : display_(display), delegate_(delegate)
{}

XrImpl::~XrImpl() = default;

bool
XrImpl::Init()
{
  wl_event_loop *wl_loop = wl_display_get_event_loop(display_);

  auto loop = std::make_unique<Loop>(wl_loop);

  peer_manager_ = zen::remote::server::CreatePeerManager(std::move(loop));
  if (!peer_manager_) {
    zn_error("Failed to create a peer manager");
    return false;
  }

  peer_manager_->on_peer_discover.Connect(
      [this](uint64_t id) { HandleRemotePeerDiscover(id); });

  peer_manager_->on_peer_lost.Connect(
      [this](uint64_t id) { HandleRemotePeerLost(id); });

  return true;
}

uint64_t
XrImpl::NextHandle()
{
  return ++next_handle_;
}

void
XrImpl::RemoveSystemIf(std::function<bool(std::unique_ptr<System> &)> pred)
{
  auto result =
      std::remove_if(systems_.begin(), systems_.end(), std::move(pred));

  std::vector<std::unique_ptr<System>> systems_to_remove(
      std::make_move_iterator(result), std::make_move_iterator(systems_.end()));

  systems_.erase(result, systems_.end());

  if (delegate_ != nullptr) {
    for (auto &system : systems_to_remove) {
      delegate_->NotifySystemRemoved(system);
    }
  }
}

void
XrImpl::HandleRemotePeerDiscover(uint64_t id)
{
  RemoveSystemIf([id](std::unique_ptr<System> &system) {
    if (system->type() != System::kRemote) {
      return false;
    }

    auto *remote_system = dynamic_cast<RemoteSystem *>(system.get());

    return remote_system != nullptr && remote_system->peer_id() == id;
  });

  auto handle = NextHandle();
  auto system = std::unique_ptr<System>(new RemoteSystem(handle, id));
  systems_.emplace_back(std::move(system));

  if (delegate_ != nullptr) {
    delegate_->NotifySystemAdded(handle);
  }
}

void
XrImpl::HandleRemotePeerLost(uint64_t id)
{
  RemoveSystemIf([id](std::unique_ptr<System> &system) {
    if (system->type() != System::kRemote) {
      return false;
    }

    auto *remote_system = dynamic_cast<RemoteSystem *>(system.get());

    return remote_system != nullptr && remote_system->peer_id() == id;
  });
}

}  // namespace zen::xr
