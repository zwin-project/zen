#pragma once

#include "zen-common/cpp-util.h"
#include "zen/xr-dispatcher.h"

namespace zen::backend::immersive::remote {

class XrDispatcher
{
 public:
  DISABLE_MOVE_AND_COPY(XrDispatcher);
  ~XrDispatcher() = default;

  static std::unique_ptr<XrDispatcher> New(
      std::shared_ptr<zen::remote::server::ISession> session);

  inline zn_xr_dispatcher *c_obj();

 private:
  XrDispatcher() = default;

  bool Init(std::shared_ptr<zen::remote::server::ISession> session);

  std::shared_ptr<zen::remote::server::IChannel> channel_;

  zn_xr_dispatcher c_obj_{};
};

inline zn_xr_dispatcher *
XrDispatcher::c_obj()
{
  return &c_obj_;
}

}  // namespace zen::backend::immersive::remote
