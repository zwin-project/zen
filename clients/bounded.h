#pragma once

#include <zen-common.h>
#include <zigen-shell-client-protocol.h>

#include "virtual-object.h"

namespace zen::client {

class Application;

class Bounded : public VirtualObject
{
 public:
  DISABLE_MOVE_AND_COPY(Bounded);
  Bounded() = delete;
  Bounded(Application *app);
  virtual ~Bounded();

  bool Init();

  inline zgn_bounded *proxy();

 private:
  Application *app_;
  zgn_bounded *proxy_ = nullptr;
};

inline zgn_bounded *
Bounded::proxy()
{
  return proxy_;
}

}  // namespace zen::client
