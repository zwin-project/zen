#pragma once

#include <zen-common.h>
#include <zigen-client-protocol.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
#include <memory>

namespace zen::client {

class Application;

class Region
{
 public:
  DISABLE_MOVE_AND_COPY(Region);
  Region() = delete;
  Region(Application *app);
  ~Region();

  bool Init();

  void AddCuboid(glm::vec3 half_size, glm::vec3 center, glm::quat quaternion);

  inline zgn_region *proxy();

 private:
  Application *app_;
  zgn_region *proxy_ = nullptr;
};

inline zgn_region *
Region::proxy()
{
  return proxy_;
}

std::unique_ptr<Region> CreateRegion(Application *app);

}  // namespace zen::client
