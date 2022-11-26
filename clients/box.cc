#include "application.h"
#include "bounded.h"
#include "cuboid.h"

using namespace zen::client;

class Box final : public Bounded
{
 public:
  Box() = delete;
  Box(Application* app) : Bounded(app), app_(app), cuboid_(new Cuboid(this)) {}

  void Frame(uint32_t time) override
  {
    animation_seed_ = (float)(time % 12000) / 12000.0f;

    cuboid_->Render(
        half_size_, glm::mat4(1), glm::vec4(1, 0, animation_seed_, 1));

    NextFrame();
    Commit();
  }

  void Configure(glm::vec3 half_size, uint32_t serial) override
  {
    half_size_ = half_size;

    AckConfigure(serial);

    cuboid_->Render(
        half_size, glm::mat4(1), glm::vec4(1, 0, animation_seed_, 1));

    if (!committed_) {
      NextFrame();
      Commit();
      committed_ = true;
    }
  }

  bool Init() { return Bounded::Init(glm::vec3(0.25, 0.25, 0.50)); }

 private:
  Application* app_;
  std::unique_ptr<Cuboid> cuboid_;
  glm::vec3 half_size_;

  float animation_seed_;  // 0 to 1
  bool committed_ = false;
};

int
main(void)
{
  Application app;

  if (!app.Init()) return EXIT_FAILURE;
  if (!app.Connect()) return EXIT_FAILURE;

  Box box(&app);
  if (!box.Init()) return EXIT_FAILURE;

  return app.Run();
}
