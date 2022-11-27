#include <linux/input.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "application.h"
#include "bounded.h"
#include "cuboid.h"
#include "region.h"

using namespace zen::client;

class Box final : public Bounded
{
 public:
  Box() = delete;
  Box(Application* app)
      : Bounded(app),
        app_(app),
        outer_cuboid_(new Cuboid(this)),
        cuboid_(new Cuboid(this))
  {}

  void DrawOuterBox()
  {
    outer_cuboid_->Render(
        half_size_, glm::mat4(1), glm::vec4(1, 0, animation_seed_, 1));
  }

  void DrawInterBox(glm::vec3* half_size = nullptr, glm::vec3* center = nullptr,
      glm::quat* quaternion = nullptr)
  {
    glm::vec3 half_size_tmp, center_tmp;
    glm::quat quaternion_tmp;
    float shortest_edge =
        glm::min(half_size_.x, glm::min(half_size_.y, half_size_.z));
    half_size_tmp = glm::vec3(shortest_edge / 4.0f);
    center_tmp = glm::vec3(shortest_edge / 4.0);
    quaternion_tmp = glm::quat(glm::vec3(M_PI / 12.0f, M_PI / 6.0f, 0));
    glm::mat4 translate = glm::translate(glm::mat4(1), center_tmp);
    glm::mat4 rotate = glm::toMat4(quaternion_tmp);

    if (enter_ && cuboid_color_ < 1) {
      cuboid_color_ += 0.04;
    }

    if (!enter_ && cuboid_color_ > 0) {
      cuboid_color_ -= 0.04;
    }

    cuboid_->Render(half_size_tmp, translate * rotate,
        glm::vec4(1 - cuboid_color_, 0, cuboid_color_, 1));

    if (half_size) *half_size = half_size_tmp;
    if (center) *center = center_tmp;
    if (quaternion) *quaternion = quaternion_tmp;
  }

  void Enter(bool enter) { enter_ = enter; }

  void Frame(uint32_t time) override
  {
    animation_seed_ = (float)(time % 12000) / 12000.0f;

    DrawOuterBox();

    DrawInterBox();

    NextFrame();
    Commit();
  }

  void Configure(glm::vec3 half_size, uint32_t serial) override
  {
    half_size_ = half_size;

    AckConfigure(serial);

    DrawOuterBox();

    glm::vec3 cuboid_half_size;
    glm::vec3 center;
    glm::quat quaternion;

    DrawInterBox(&cuboid_half_size, &center, &quaternion);

    auto region = CreateRegion(app_);

    region->AddCuboid(cuboid_half_size, center, quaternion);

    SetRegion(region.get());

    if (!committed_) {
      NextFrame();
      Commit();
      committed_ = true;
    }
  }

  bool Init() { return Bounded::Init(glm::vec3(0.50, 0.25, 0.25)); }

 private:
  Application* app_;
  std::unique_ptr<Cuboid> outer_cuboid_;
  std::unique_ptr<Cuboid> cuboid_;
  glm::vec3 half_size_;

  float animation_seed_;  // 0 to 1
  bool committed_ = false;
  bool enter_ = false;
  float cuboid_color_ = 0;
};

class BoxApp final : public Application
{
 public:
  BoxApp() : box_(this) {}

  bool Init()
  {
    if (!Application::Init()) return false;

    if (!Application::Connect()) return false;

    if (!box_.Init()) return false;

    return true;
  }

 protected:
  void RayEnter(uint32_t /*serial*/, VirtualObject* /*virtual_object*/,
      glm::vec3 /*origin*/, glm::vec3 /*direction*/) override
  {
    box_.Enter(true);
  };

  void RayLeave(uint32_t /*serial*/, VirtualObject* /*virtual_object*/) override
  {
    box_.Enter(false);
  };

  void RayButton(uint32_t serial, uint32_t /*time*/, uint32_t button,
      uint32_t state) override
  {
    if (button == BTN_LEFT && state == ZGN_RAY_BUTTON_STATE_PRESSED) {
      box_.Move(serial);
    }
  };

 private:
  Box box_;
};

int
main(void)
{
  BoxApp app;

  if (!app.Init()) return EXIT_FAILURE;

  return app.Run();
}
