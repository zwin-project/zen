#include "xr-system.h"

#include "loop.h"
#include "virtual-object.h"
#include "xr-dispatcher.h"
#include "xr-system-manager.h"
#include "zen-common/log.h"
#include "zen-common/signal.h"

namespace zen::backend::immersive::remote {

XrSystem::XrSystem(std::shared_ptr<zen::remote::server::IPeer> peer,
    wl_display *display, XrSystemManager *xr_system_manager)
    : peer_(std::move(peer)),
      display_(display),
      xr_system_manager_(xr_system_manager)
{}

XrSystem::~XrSystem()
{
  zn_signal_emit_mutable(&c_obj_.events.destroy, nullptr);
}

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
  c_obj_.impl_data = this;
  c_obj_.impl = &c_implementation_;
  c_obj_.state = ZN_XR_SYSTEM_SESSION_STATE_AVAILABLE;
  wl_signal_init(&c_obj_.events.session_state_changed);
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

  Kill();

  xr_system_manager_->RemoveDeadXrSystem();  // Will destroy this XrSystem
}

void
XrSystem::Kill()
{
  if (c_obj_.state == ZN_XR_SYSTEM_SESSION_STATE_AVAILABLE) {
    c_obj_.state = ZN_XR_SYSTEM_SESSION_STATE_DEAD;
    zn_signal_emit_mutable(&c_obj_.events.session_state_changed, nullptr);
    return;
  }

  if (c_obj_.state == ZN_XR_SYSTEM_SESSION_STATE_FOCUS) {
    c_obj_.state = ZN_XR_SYSTEM_SESSION_STATE_VISIBLE;
    zn_signal_emit_mutable(&c_obj_.events.session_state_changed, nullptr);
  }

  if (c_obj_.state == ZN_XR_SYSTEM_SESSION_STATE_VISIBLE) {
    c_obj_.state = ZN_XR_SYSTEM_SESSION_STATE_SYNCHRONIZED;
    zn_signal_emit_mutable(&c_obj_.events.session_state_changed, nullptr);
  }

  if (c_obj_.state == ZN_XR_SYSTEM_SESSION_STATE_SYNCHRONIZED) {
    c_obj_.state = ZN_XR_SYSTEM_SESSION_STATE_DEAD;

    session_.reset();
    high_priority_dispatcher_.reset();
    default_dispatcher_.reset();
    c_obj_.high_priority_dispatcher = nullptr;
    c_obj_.default_dispatcher = nullptr;

    zn_signal_emit_mutable(&c_obj_.events.session_state_changed, nullptr);
  }
}

void
XrSystem::Connect()
{
  if (c_obj_.state != ZN_XR_SYSTEM_SESSION_STATE_AVAILABLE) {
    zn_warn("Tried to Connect to %s but the current state(%d) is invalid.",
        peer_->host().c_str(), c_obj_.state);
    return;
  }

  auto loop = std::make_unique<Loop>(wl_display_get_event_loop(display_));

  auto new_session = zen::remote::server::CreateSession(std::move(loop));

  if (!new_session->Connect(peer_)) {
    zn_warn("Failed to start new session");
    return;
  }

  auto high_priority_dispatcher = XrDispatcher::New(new_session, display_);
  if (!high_priority_dispatcher) {
    zn_error("Failed to create a remote XrDispatcher");
  }

  auto default_dispatcher = XrDispatcher::New(new_session, display_);
  if (!default_dispatcher) {
    zn_error("Failed to create a remote XrDispatcher");
  }

  zn_info("Connected to peer %ld", peer_->id());

  new_session->on_disconnect.Connect([this]() { HandleDisconnect(); });

  session_ = new_session;

  c_obj_.state = ZN_XR_SYSTEM_SESSION_STATE_SYNCHRONIZED;
  high_priority_dispatcher_ = std::move(high_priority_dispatcher);
  default_dispatcher_ = std::move(default_dispatcher);
  c_obj_.high_priority_dispatcher = high_priority_dispatcher_->c_obj();
  c_obj_.default_dispatcher = default_dispatcher_->c_obj();

  zn_signal_emit_mutable(&c_obj_.events.session_state_changed, nullptr);

  c_obj_.state = ZN_XR_SYSTEM_SESSION_STATE_VISIBLE;
  zn_signal_emit_mutable(&c_obj_.events.session_state_changed, nullptr);

  c_obj_.state = ZN_XR_SYSTEM_SESSION_STATE_FOCUS;
  zn_signal_emit_mutable(&c_obj_.events.session_state_changed, nullptr);
}

}  // namespace zen::backend::immersive::remote
