#include "xr-system.h"

#include "zen-common/log.h"

namespace zen::backend::immersive::remote {

XrSystem::XrSystem(uint64_t peer_id) : peer_id_(peer_id) {}

XrSystem::~XrSystem() { wl_signal_emit(&c_obj_.events.destroy, nullptr); }

std::unique_ptr<XrSystem>
XrSystem::New(uint64_t peer_id)
{
  auto self = std::unique_ptr<XrSystem>(new XrSystem(peer_id));
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
  wl_signal_init(&c_obj_.events.destroy);

  return true;
}

}  // namespace zen::backend::immersive::remote
