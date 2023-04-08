#pragma once

#include "zen-common/cpp-util.h"
#include "zen/xr-system.h"

namespace zen::backend::immersive::remote {

class XrSystemManager;
class XrDispatcher;

class XrSystem
{
 public:
  DISABLE_MOVE_AND_COPY(XrSystem);
  ~XrSystem();

  static std::unique_ptr<XrSystem> New(
      std::shared_ptr<zen::remote::server::IPeer> peer, wl_display *display,
      XrSystemManager *xr_system_manager);

  void Kill();

  inline uint64_t peer_id() const;

  inline zn_xr_system *c_obj();

  inline bool is_alive() const;

  inline bool is_connected() const;

 private:
  XrSystem(std::shared_ptr<zen::remote::server::IPeer> peer,
      wl_display *display, XrSystemManager *xr_system_manager);

  bool Init();

  void Connect();

  static void HandleConnect(zn_xr_system *c_obj);

  void HandleDisconnect();

  static const zn_xr_system_interface c_implementation_;

  std::shared_ptr<zen::remote::server::IPeer> peer_;  // @nonnull

  wl_display *display_;  // @nonnull, @outlive

  XrSystemManager *xr_system_manager_;  // @nonnull, @outlive

  // These objects are null when state is AVAILABLE or DEAD, not null otherwise.
  std::shared_ptr<zen::remote::server::ISession> session_;
  std::unique_ptr<XrDispatcher> high_priority_dispatcher_;
  std::unique_ptr<XrDispatcher> default_dispatcher_;

  zn_xr_system c_obj_{};
};

inline uint64_t
XrSystem::peer_id() const
{
  return peer_->id();
}

inline zn_xr_system *
XrSystem::c_obj()
{
  return &c_obj_;
}

inline bool
XrSystem::is_alive() const
{
  return c_obj_.state != ZN_XR_SYSTEM_SESSION_STATE_DEAD;
}

inline bool
XrSystem::is_connected() const
{
  uint32_t connected = ZN_XR_SYSTEM_SESSION_STATE_SYNCHRONIZED |
                       ZN_XR_SYSTEM_SESSION_STATE_VISIBLE |
                       ZN_XR_SYSTEM_SESSION_STATE_FOCUS;

  return (c_obj_.state & connected) != 0;
}

}  // namespace zen::backend::immersive::remote
