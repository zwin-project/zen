#include "bounded.h"

#include "application.h"

namespace zen::client {

bool
Bounded::Init()
{
  if (!VirtualObject::Init()) return false;

  proxy_ = zgn_shell_get_bounded(app_->shell(), VirtualObject::proxy());
  if (proxy_ == nullptr) {
    zn_error("Failed to create bounded proxy");
    return false;
  }

  return true;
}

Bounded::Bounded(Application *app) : VirtualObject(app), app_(app) {}

Bounded::~Bounded()
{
  if (proxy_) {
    zgn_bounded_destroy(proxy_);
  }
}

}  // namespace zen::client
