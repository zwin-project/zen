#include <string.h>
#include <wayland-client.h>
#include <zukou.h>

namespace zukou {

static void
cuboid_window_configure(void *data,
    [[maybe_unused]] struct zgn_cuboid_window *zgn_cuboid_window,
    uint32_t serial, struct wl_array *half_size_array,
    struct wl_array *quaternion_array)
{
  CuboidWindow *cuboid_window = (CuboidWindow *)data;
  glm::vec3 half_size = glm_vec3_from_wl_array(half_size_array);
  glm::quat quaternion = glm_quat_from_wl_array(quaternion_array);

  cuboid_window->Configure(serial, half_size, quaternion);
}

static void
cuboid_window_moved(void *data,
    [[maybe_unused]] struct zgn_cuboid_window *zgn_cuboid_window,
    struct wl_array *face_direction_array)
{
  CuboidWindow *cuboid_window = (CuboidWindow *)data;
  glm::vec3 face_direction = glm_vec3_from_wl_array(face_direction_array);

  cuboid_window->Moved(face_direction);
}

static const struct zgn_cuboid_window_listener cuboid_window_listener = {
    cuboid_window_configure,
    cuboid_window_moved,
};

CuboidWindow::CuboidWindow(App *app, glm::vec3 half_size, glm::quat quaternion)
    : VirtualObject(app)
{
  struct wl_array half_size_array, quaternion_array;

  wl_array_init(&half_size_array);
  wl_array_init(&quaternion_array);
  glm_vec3_to_wl_array(half_size, &half_size_array);
  glm_quat_to_wl_array(quaternion, &quaternion_array);

  cuboid_window_ = zgn_shell_get_cuboid_window(app->shell(),
      this->virtual_object(), &half_size_array, &quaternion_array);

  wl_array_release(&half_size_array);

  zgn_cuboid_window_add_listener(cuboid_window_, &cuboid_window_listener, this);

  half_size_ = glm::vec3(0.0f);
  quaternion_ = glm::quat();
}

CuboidWindow::CuboidWindow(App *app, glm::vec3 half_size)
    : CuboidWindow(app, half_size, glm::quat())
{}

CuboidWindow::~CuboidWindow() { zgn_cuboid_window_destroy(cuboid_window_); }

void
CuboidWindow::Configure(
    uint32_t serial, glm::vec3 half_size, glm::quat quaternion)
{
  zgn_cuboid_window_ack_configure(cuboid_window(), serial);
  half_size_ = half_size;
  quaternion_ = quaternion;
}

void
CuboidWindow::Moved([[maybe_unused]] glm::vec3 face_direction)
{}

void
CuboidWindow::Rotate(glm::quat quaternion)
{
  struct wl_array array;

  wl_array_init(&array);
  glm_quat_to_wl_array(quaternion, &array);

  zgn_cuboid_window_rotate(cuboid_window(), &array);

  wl_array_release(&array);
}
}  // namespace zukou
