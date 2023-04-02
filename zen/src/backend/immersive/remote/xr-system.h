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

  inline uint64_t peer_id() const;

  inline zn_xr_system *c_obj();

  inline void set_unavailable();

  inline bool is_alive() const;

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

  // Null when status is NOT_CONNECTED. Not null otherwise.
  std::shared_ptr<zen::remote::server::ISession> session_;

  std::unique_ptr<XrDispatcher> high_priority_dispatcher_;
  std::unique_ptr<XrDispatcher> default_dispatcher_;

  // True if available to create a new session.
  bool is_available_ = true;

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

inline void
XrSystem::set_unavailable()
{
  is_available_ = false;
}

inline bool
XrSystem::is_alive() const
{
  return is_available_ ||
         c_obj_.status == ZN_XR_SYSTEM_SESSION_STATUS_CONNECTED;
}

}  // namespace zen::backend::immersive::remote
