#pragma once

#include "backend/immersive/xr-system-manager.h"
#include "zen-common/cpp-util.h"

namespace zen::backend::immersive::remote {

class XrSystem;

class XrSystemManager
{
 public:
  DISABLE_MOVE_AND_COPY(XrSystemManager);
  ~XrSystemManager() = default;

  void RemoveDeadXrSystem();

 private:
  friend struct zn_xr_system_manager * ::zn_xr_system_manager_create_remote(
      struct wl_display *display);

  explicit XrSystemManager(wl_display *display);

  bool Init();

  void HandleRemotePeerDiscover(uint64_t peer_id);

  void HandleRemotePeerLost(uint64_t peer_id);

  static void HandleDestroy(zn_xr_system_manager *c_obj);

  static const zn_xr_system_manager_interface c_implementation_;

  zn_xr_system_manager c_obj_{};

  wl_display *display_;  // @nonnull, @outlive

  std::unique_ptr<zen::remote::server::IPeerManager> peer_manager_;  // @nonnull

  std::vector<std::unique_ptr<XrSystem>> xr_systems_;
};

}  // namespace zen::backend::immersive::remote
