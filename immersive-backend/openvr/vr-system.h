#ifndef ZEN_OPENVR_BACKEND_VR_SYSTEM_H
#define ZEN_OPENVR_BACKEND_VR_SYSTEM_H

#include <openvr/openvr.h>

#include "zen-common.h"

namespace zen {

class VrSystem
{
 public:
  VrSystem() = default;
  ~VrSystem() = default;
  DISABLE_MOVE_AND_COPY(VrSystem)

  bool Connect();
  void Disconnect();
};

}  // namespace zen

#endif  //  ZEN_OPENVR_BACKEND_VR_SYSTEM_H
