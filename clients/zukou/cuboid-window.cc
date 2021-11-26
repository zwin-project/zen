#include <string.h>
#include <wayland-client.h>
#include <zukou.h>

namespace zukou {

static void
cuboid_window_configure(void *data, struct zgn_cuboid_window *zgn_cuboid_window,
    uint32_t serial, struct wl_array *half_size_array)
{
  (void)zgn_cuboid_window;
  CuboidWindow *cuboid_window = (CuboidWindow *)data;
  glm::vec3 half_size = glm_vec3_from_wl_array(half_size_array);

  cuboid_window->Configure(serial, half_size);
}

static const struct zgn_cuboid_window_listener cuboid_window_listener = {
    cuboid_window_configure,
};

CuboidWindow::CuboidWindow(App *app, glm::vec3 half_size) : VirtualObject(app)
{
  struct wl_array half_size_array;

  wl_array_init(&half_size_array);
  glm_vec3_to_wl_array(half_size, &half_size_array);

  cuboid_window_ = zgn_shell_get_cuboid_window(
      app->shell(), this->virtual_object(), &half_size_array);

  wl_array_release(&half_size_array);

  zgn_cuboid_window_add_listener(cuboid_window_, &cuboid_window_listener, this);

  half_size_ = glm::vec3(0.0f);
}

CuboidWindow::~CuboidWindow() { zgn_cuboid_window_destroy(cuboid_window_); }

void
CuboidWindow::Configure(uint32_t serial, glm::vec3 half_size)
{
  zgn_cuboid_window_ack_configure(cuboid_window(), serial);
  half_size_ = half_size;
}
}  // namespace zukou
