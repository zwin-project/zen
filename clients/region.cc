#include "region.h"

#include "application.h"
#include "array.h"

namespace zen::client {

bool
Region::Init()
{
  proxy_ = zgn_compositor_create_region(app_->compositor());
  if (proxy_ == nullptr) {
    zn_error("Failed to create region proxy");
    return false;
  }

  return true;
}

void
Region::AddCuboid(glm::vec3 half_size, glm::vec3 center, glm::quat quaternion)
{
  struct wl_array half_size_array;
  struct wl_array center_array;
  struct wl_array quaternion_array;

  to_array(half_size, &half_size_array);
  to_array(center, &center_array);
  to_array(quaternion, &quaternion_array);

  zgn_region_add_cuboid(
      proxy_, &half_size_array, &center_array, &quaternion_array);

  wl_array_release(&half_size_array);
  wl_array_release(&center_array);
  wl_array_release(&quaternion_array);
}

Region::Region(Application *app) : app_(app) {}

Region::~Region()
{
  if (proxy_) {
    zgn_region_destroy(proxy_);
  }
}

std::unique_ptr<Region>
CreateRegion(Application *app)
{
  auto region = std::make_unique<Region>(app);

  if (!region->Init()) {
    return std::unique_ptr<Region>();
  }

  return region;
}

}  // namespace zen::client
