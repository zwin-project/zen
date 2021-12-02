#include <zukou.h>

namespace zukou {
VirtualObject::VirtualObject(App *app)
{
  app_ = app;
  virtual_object_ = zgn_compositor_create_virtual_object(app->compositor());
  frame_callback_ = nullptr;
  wl_proxy_set_user_data((wl_proxy *)virtual_object_, this);
}

VirtualObject::~VirtualObject()
{
  if (frame_callback_) wl_callback_destroy(frame_callback_);
  zgn_virtual_object_destroy(virtual_object_);
}

static void
frame_callback_handler(
    void *data, struct wl_callback *callback, uint32_t callback_data)
{
  VirtualObject *virtual_object = (VirtualObject *)data;
  wl_callback_destroy(callback);
  virtual_object->Frame(callback_data);
}

static const struct wl_callback_listener frame_callback_listener = {
    frame_callback_handler,
};

void
VirtualObject::NextFrame()
{
  frame_callback_ = zgn_virtual_object_frame(virtual_object_);
  wl_callback_add_listener(frame_callback_, &frame_callback_listener, this);
  zgn_virtual_object_commit(virtual_object_);
}

void
VirtualObject::Commit()
{
  zgn_virtual_object_commit(virtual_object_);
}

void
VirtualObject::Frame(uint32_t time)
{
  (void)time;
}

void
VirtualObject::RayEnter(uint32_t serial, glm::vec3 origin, glm::vec3 direction)
{
  (void)serial;
  (void)origin;
  (void)direction;
}

void
VirtualObject::RayLeave(uint32_t serial)
{
  (void)serial;
}

void
VirtualObject::RayMotion(uint32_t time, glm::vec3 origin, glm::vec3 direction)
{
  (void)time;
  (void)origin;
  (void)direction;
}

}  // namespace zukou
