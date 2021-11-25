#ifndef ZEN_CLIENT_ZUKOU_H
#define ZEN_CLIENT_ZUKOU_H

#include <wayland-client.h>
#include <zigen-client-protocol.h>
#include <zigen-opengl-client-protocol.h>
#include <zigen-shell-client-protocol.h>

namespace zukou {
class App
{
 public:
  App();
  ~App();

  bool Connect(const char *socket);
  void GlobalRegistryHandler(struct wl_registry *registry, uint32_t id,
      const char *interface, uint32_t version);
  void GlobalRegistryRemover(struct wl_registry *registry, uint32_t id);
  void ShmFormat(struct wl_shm *wl_shm, uint32_t format);
  bool Run();
  void Terminate();
  inline struct wl_display *display();
  inline struct wl_registry *registry();
  inline struct zgn_compositor *compositor();
  inline struct zgn_shell *shell();
  inline struct wl_shm *shm();
  inline struct zgn_opengl *opengl();

 private:
  struct wl_display *display_;
  struct wl_registry *registry_;
  struct zgn_compositor *compositor_;
  struct zgn_shell *shell_;
  struct wl_shm *shm_;
  struct zgn_opengl *opengl_;
  bool running_ = false;
};

inline struct wl_display *
App::display()
{
  return display_;
}

inline struct wl_registry *
App::registry()
{
  return registry_;
}

inline struct zgn_compositor *
App::compositor()
{
  return compositor_;
}

inline struct zgn_shell *
App::shell()
{
  return shell_;
}

inline struct wl_shm *
App::shm()
{
  return shm_;
}

inline struct zgn_opengl *
App::opengl()
{
  return opengl_;
}

class VirtualObject
{
 public:
  VirtualObject(App *app);
  virtual ~VirtualObject();
  void NextFrame();
  void Commit();
  virtual void Frame(uint32_t time);
  inline App *app();
  inline struct zgn_virtual_object *virtual_object();
  inline struct wl_callback *frame_callback();

 private:
  App *app_;
  struct zgn_virtual_object *virtual_object_;
  struct wl_callback *frame_callback_;
};

inline App *
VirtualObject::app()
{
  return app_;
}

inline struct zgn_virtual_object *
VirtualObject::virtual_object()
{
  return virtual_object_;
}

inline struct wl_callback *
VirtualObject::frame_callback()
{
  return frame_callback_;
}

class OpenGLComponent
{
 public:
  OpenGLComponent(App *app, VirtualObject *virtual_object);
  inline App *app();
  inline struct zgn_opengl_component *component();

 private:
  App *app_;
  struct zgn_opengl_component *component_;
};

inline App *
OpenGLComponent::app()
{
  return app_;
}

inline struct zgn_opengl_component *
OpenGLComponent::component()
{
  return component_;
}

}  // namespace zukou

#endif  //  ZEN_CLIENT_ZUKOU_H
