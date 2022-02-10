#include <wayland-client.h>
#include <zukou.h>

namespace zukou {

Background::Background(App *app) : VirtualObject(app)
{
  background_ = zgn_shell_get_background(app->shell(), this->virtual_object());
}

Background::~Background() { zgn_background_destroy(background_); }

}  // namespace zukou
