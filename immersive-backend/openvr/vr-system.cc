#include "vr-system.h"

#include <openvr/openvr.h>

namespace zen {

bool
VrSystem::Connect()
{
  auto init_error = vr::VRInitError_None;

  (void)vr::VR_Init(&init_error, vr::VRApplication_Scene);
  if (init_error != vr::VRInitError_None) {
    zn_warn("Failed to init OpenVR: %s",
        vr::VR_GetVRInitErrorAsEnglishDescription(init_error));
    goto err;
  }

  return true;

err:
  return false;
}

void
VrSystem::Disconnect()
{
  vr::VR_Shutdown();
}

}  // namespace zen
