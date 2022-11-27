#include "bounded.h"

#include <cstring>
#include <iostream>

#include "application.h"
#include "array.h"
#include "region.h"

namespace zen::client {

bool
Bounded::Init(glm::vec3 half_size)
{
  if (!VirtualObject::Init()) return false;

  struct wl_array array;
  to_array(half_size, &array);

  proxy_ = zgn_shell_get_bounded(app_->shell(), VirtualObject::proxy(), &array);
  if (proxy_ == nullptr) {
    zn_error("Failed to create bounded proxy");
    return false;
  }

  zgn_bounded_add_listener(proxy_, &Bounded::listener, this);

  wl_array_release(&array);

  return true;
}

void
Bounded::AckConfigure(uint32_t serial)
{
  zgn_bounded_ack_configure(proxy_, serial);
}

void
Bounded::SetRegion(Region *region)
{
  zgn_bounded_set_region(proxy_, region->proxy());
}

const struct zgn_bounded_listener Bounded::listener = {
    Bounded::Configure,
};

void
Bounded::Configure(void *data, struct zgn_bounded * /*zgn_bounded*/,
    struct wl_array *half_size_array, uint32_t serial)
{
  auto self = static_cast<Bounded *>(data);

  glm::vec3 half_size;
  to_vec3(half_size_array, half_size);

  self->Configure(half_size, serial);
}

void
Bounded::Move(uint32_t serial)
{
  zgn_bounded_move(proxy_, app_->seat(), serial);
}

Bounded::Bounded(Application *app) : VirtualObject(app), app_(app) {}

Bounded::~Bounded()
{
  if (proxy_) {
    zgn_bounded_destroy(proxy_);
  }
}

}  // namespace zen::client
