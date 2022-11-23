#pragma once

#include <zen-common.h>
#include <zigen-shell-client-protocol.h>

#include <glm/vec3.hpp>

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

  bool Init(glm::vec3 half_size);

  virtual void Configure(glm::vec3 /*half_size*/, uint32_t /*serial*/){};

  void AckConfigure(uint32_t serial);

  inline zgn_bounded *proxy();

 private:
  static const struct zgn_bounded_listener listener;
  static void Configure(void *data, struct zgn_bounded *zgn_bounded,
      struct wl_array *half_size, uint32_t serial);

  Application *app_;
  zgn_bounded *proxy_ = nullptr;
};

inline zgn_bounded *
Bounded::proxy()
{
  return proxy_;
}

}  // namespace zen::client
