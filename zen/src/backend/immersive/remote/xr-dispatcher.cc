#include "xr-dispatcher.h"

#include <utility>

#include "zen-common/log.h"

namespace zen::backend::immersive::remote {

std::unique_ptr<XrDispatcher>
XrDispatcher::New(std::shared_ptr<zen::remote::server::ISession> session)
{
  auto self = std::unique_ptr<XrDispatcher>(new XrDispatcher());
  if (!self) {
    zn_error("Failed to allocate memory");
    return nullptr;
  }

  if (!self->Init(std::move(session))) {
    zn_error("Failed to initialize remote XrDispatcher");
    return nullptr;
  }

  return self;
}

bool
XrDispatcher::Init(std::shared_ptr<zen::remote::server::ISession> session)
{
  channel_ = zen::remote::server::CreateChannel(session);
  if (!channel_) {
    zn_error("Failed to create a remote channel");
    return false;
  }

  c_obj_.impl_data = this;

  return true;
}

}  // namespace zen::backend::immersive::remote
