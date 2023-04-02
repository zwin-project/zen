#include "xr-system.h"

#include "loop.h"
#include "xr-dispatcher.h"
#include "xr-system-manager.h"
#include "zen-common/log.h"

namespace zen::backend::immersive::remote {

XrSystem::XrSystem(std::shared_ptr<zen::remote::server::IPeer> peer,
    wl_display *display, XrSystemManager *xr_system_manager)
    : peer_(std::move(peer)),
      display_(display),
      xr_system_manager_(xr_system_manager)
{}

XrSystem::~XrSystem() { wl_signal_emit(&c_obj_.events.destroy, nullptr); }

std::unique_ptr<XrSystem>
XrSystem::New(std::shared_ptr<zen::remote::server::IPeer> peer,
    wl_display *display, XrSystemManager *xr_system_manager)
{
  auto self = std::unique_ptr<XrSystem>(
      new XrSystem(std::move(peer), display, xr_system_manager));
  if (!self) {
    zn_error("Failed to allocate memory");
    return nullptr;
  }

  if (!self->Init()) {
    zn_error("Failed to initialize remote XrSystem");
    return nullptr;
  }

  return self;
}

bool
XrSystem::Init()
{
  high_priority_dispatcher_ = XrDispatcher::New(session_);
  if (!high_priority_dispatcher_) {
    zn_error("Failed to create a remote XrDispatcher");
    return false;
  }

  default_dispatcher_ = XrDispatcher::New(session_);
  if (!default_dispatcher_) {
    zn_error("Failed to create a remote XrDispatcher");
    return false;
  }

  c_obj_.impl_data = this;
  c_obj_.impl = &c_implementation_;
  c_obj_.status = ZN_XR_SYSTEM_SESSION_STATUS_NOT_CONNECTED;
  c_obj_.high_priority_dispatcher = high_priority_dispatcher_->c_obj();
  c_obj_.default_dispatcher = default_dispatcher_->c_obj();
  wl_signal_init(&c_obj_.events.session_status_changed);
  wl_signal_init(&c_obj_.events.destroy);

  return true;
}

const zn_xr_system_interface XrSystem::c_implementation_ = {
    XrSystem::HandleConnect,
};

void
XrSystem::HandleConnect(zn_xr_system *c_obj)
{
  auto *self = static_cast<XrSystem *>(c_obj->impl_data);

  self->Connect();
}

void
XrSystem::HandleDisconnect()
{
  zn_info("Disconnected from peer %ld", peer_->id());

  c_obj_.status = ZN_XR_SYSTEM_SESSION_STATUS_NOT_CONNECTED;

  wl_signal_emit(&c_obj_.events.session_status_changed, nullptr);

  xr_system_manager_->RemoveDeadXrSystem();  // Will destroy this XrSystem
}

void
XrSystem::Connect()
{
  if (session_) {
    return;
  }

  auto loop = std::make_unique<Loop>(wl_display_get_event_loop(display_));

  auto new_session = zen::remote::server::CreateSession(std::move(loop));

  if (!new_session->Connect(peer_)) {
    zn_warn("Failed to start new session");
    return;
  }

  zn_info("Connected to peer %ld", peer_->id());

  set_unavailable();

  new_session->on_disconnect.Connect([this]() { HandleDisconnect(); });

  session_ = new_session;

  c_obj_.status = ZN_XR_SYSTEM_SESSION_STATUS_CONNECTED;

  wl_signal_emit(&c_obj_.events.session_status_changed, nullptr);
}

}  // namespace zen::backend::immersive::remote
