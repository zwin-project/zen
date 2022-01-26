#ifndef ZEN_CLIENT_ZUKOU_H
#define ZEN_CLIENT_ZUKOU_H

#include <sys/epoll.h>
#include <wayland-client.h>
#include <zigen-client-protocol.h>
#include <zigen-opengl-client-protocol.h>
#include <zigen-shell-client-protocol.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>

namespace zukou {

class DataOffer;
class DataSource;
class VirtualObject;

class App
{
 public:
  App();
  virtual ~App();

  bool Connect(const char *socket);
  void GlobalRegistryHandler(struct wl_registry *registry, uint32_t id,
      const char *interface, uint32_t version);
  void GlobalRegistryRemover(struct wl_registry *registry, uint32_t id);
  void ShmFormat(struct wl_shm *wl_shm, uint32_t format);
  void DataDeviceDataOffer(
      struct zgn_data_device *data_device, struct zgn_data_offer *id);
  void DataDeviceEnter(struct zgn_data_device *data_device, uint32_t serial,
      struct zgn_virtual_object *virtual_object, glm::vec3 origin,
      glm::vec3 direction, struct zgn_data_offer *id);
  void DataDeviceLeave(struct zgn_data_device *data_device);
  void DataDeviceMotion(struct zgn_data_device *data_device, uint32_t time,
      glm::vec3 origin, glm::vec3 direction);
  void DataDeviceDrop(struct zgn_data_device *data_device);
  void Capabilities(struct zgn_seat *seat, uint32_t capability);
  void RayEnter(struct zgn_ray *ray, uint32_t serial,
      struct zgn_virtual_object *virtual_object, glm::vec3 origin,
      glm::vec3 direction);
  void RayLeave(struct zgn_ray *ray, uint32_t serial,
      struct zgn_virtual_object *virtual_object);
  void RayMotion(struct zgn_ray *ray, uint32_t time, glm::vec3 origin,
      glm::vec3 direction);
  void RayButton(struct zgn_ray *ray, uint32_t serial, uint32_t time,
      uint32_t button, enum zgn_ray_button_state state);
  void StartDrag(VirtualObject *virtual_object,
      std::vector<std::string> *mime_types, uint32_t enter_serial);

  void DestroyDataSource();
  void DestroyDataOffer();

  virtual void DataSourceTarget(const char *mime_type);
  virtual void DataSourceSend(const char *mime_type, int32_t fd);
  virtual void DataSourceDndDropPerformed();
  virtual void DataSourceAction(uint32_t action);

  bool Run();
  void Terminate();
  inline struct wl_display *display();
  inline struct wl_registry *registry();
  inline struct zgn_compositor *compositor();
  inline struct zgn_data_device_manager *data_device_manager();
  inline struct zgn_data_device *data_device();
  inline DataOffer *data_offer();
  inline DataSource *data_source();
  inline struct zgn_seat *seat();
  inline struct zgn_shell *shell();
  inline struct wl_shm *shm();
  inline struct zgn_opengl *opengl();
  inline int epoll_fd();

 private:
  struct wl_display *display_;
  struct wl_registry *registry_;
  struct zgn_compositor *compositor_;
  struct zgn_data_device_manager *data_device_manager_;
  struct zgn_data_device *data_device_;  // nonnull
  DataOffer *data_offer_;                // nullable
  DataSource *data_source_;              // nullable
  struct zgn_seat *seat_;
  struct zgn_shell *shell_;
  struct wl_shm *shm_;
  struct zgn_opengl *opengl_;
  struct zgn_ray *ray_;  // nullable
  bool running_ = false;

  int epoll_fd_;

  struct zgn_virtual_object *ray_focus_virtual_object_;
  struct zgn_virtual_object *data_device_focus_virtual_object_;
  struct zgn_virtual_object *data_offer_focus_virtual_object_;
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

inline struct zgn_data_device_manager *
App::data_device_manager()
{
  return data_device_manager_;
}

inline struct zgn_data_device *
App::data_device()
{
  return data_device_;
}

inline DataOffer *
App::data_offer()
{
  return data_offer_;
}

inline DataSource *
App::data_source()
{
  return data_source_;
}

inline struct zgn_seat *
App::seat()
{
  return seat_;
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

inline int
App::epoll_fd()
{
  return epoll_fd_;
}

struct ColorBGRA {
  uint8_t b, g, r, a;
};

class Buffer
{
 public:
  Buffer(App *app, size_t size);
  Buffer(App *app, int32_t stride, int32_t height, int32_t width,
      enum wl_shm_format format);
  ~Buffer();
  inline void *data();

 protected:
  size_t size_;
  int fd_;
  void *data_;
  struct wl_shm_pool *pool_;
  struct wl_buffer *wl_buffer_;
};

inline void *
Buffer::data()
{
  return data_;
}

class VirtualObject
{
 public:
  VirtualObject(App *app);
  virtual ~VirtualObject();
  void NextFrame();
  void Commit();
  virtual void Frame(uint32_t time);
  virtual void DataDeviceEnter(uint32_t serial, glm::vec3 origin,
      glm::vec3 direction, struct zgn_data_offer *id);
  virtual void DataDeviceLeave();
  virtual void DataDeviceMotion(
      uint32_t time, glm::vec3 origin, glm::vec3 direction);
  virtual void DataDeviceDrop();
  virtual void RayEnter(uint32_t serial, glm::vec3 origin, glm::vec3 direction);
  virtual void RayLeave(uint32_t serial);
  virtual void RayMotion(uint32_t time, glm::vec3 origin, glm::vec3 direction);
  virtual void RayButton(uint32_t serial, uint32_t time, uint32_t button,
      enum zgn_ray_button_state state);
  inline App *app();
  inline struct zgn_virtual_object *virtual_object();
  inline struct wl_callback *frame_callback();

 protected:
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

class CuboidWindow : public VirtualObject
{
 public:
  CuboidWindow(App *app, glm::vec3 half_size);
  CuboidWindow(App *app, glm::vec3 half_size, glm::quat quaternion);
  virtual ~CuboidWindow();
  virtual void Configure(
      uint32_t serial, glm::vec3 half_size, glm::quat quaternion);
  virtual void Moved(glm::vec3 face_direction);
  void Rotate(glm::quat quaternion);
  struct zgn_cuboid_window *cuboid_window();
  glm::vec3 half_size();
  glm::quat quaternion();

 protected:
  struct zgn_cuboid_window *cuboid_window_;
  glm::vec3 half_size_;
  glm::quat quaternion_;
};

inline struct zgn_cuboid_window *
CuboidWindow::cuboid_window()
{
  return cuboid_window_;
}

inline glm::vec3
CuboidWindow::half_size()
{
  return half_size_;
}

inline glm::quat
CuboidWindow::quaternion()
{
  return quaternion_;
}

class Task
{
 public:
  virtual ~Task(){};
  virtual void Done(){};
};

class DndTask : Task
{
 public:
  virtual ~DndTask(){};
  virtual void Done(){};
  int fd;
};

class DataOffer
{
 public:
  DataOffer(struct zgn_data_offer *data_offer, App *app);
  ~DataOffer();
  void Offer(const char *mime_type);
  void SourceActions(uint32_t source_actions);
  void Action(uint32_t dnd_action);
  void Receive(std::string mime_type, DndTask *task);
  void Accept(uint32_t serial, std::string mime_type);
  void SetActions(uint32_t actions, uint32_t preffered_action);

  inline struct zgn_data_offer *data_offer();
  inline std::vector<std::string> *mime_types();
  inline uint32_t source_actions();
  inline uint32_t dnd_action();

 private:
  App *app_;
  struct zgn_data_offer *data_offer_;
  std::vector<std::string> mime_types_;
  uint32_t source_actions_;
  uint32_t dnd_action_;
};

inline struct zgn_data_offer *
DataOffer::data_offer()
{
  return data_offer_;
}

inline std::vector<std::string> *
DataOffer::mime_types()
{
  return &mime_types_;
}

inline uint32_t
DataOffer::source_actions()
{
  return source_actions_;
}

inline uint32_t
DataOffer::dnd_action()
{
  return dnd_action_;
}

class DataSource
{
 public:
  DataSource(App *app);
  ~DataSource();
  void Target(const char *mime_type);
  void Send(const char *mime_type, int32_t fd);
  void Cancelled();
  void DndDropPerformed();
  void DndFinished();
  void Action(uint32_t dnd_action);

  inline struct zgn_data_source *data_source();

 private:
  App *app_;
  struct zgn_data_source *data_source_;
};

inline struct zgn_data_source *
DataSource::data_source()
{
  return data_source_;
}

class OpenGLTexture : public Buffer
{
 public:
  OpenGLTexture(App *app, int32_t height, int32_t width);
  void BufferUpdated();
  inline struct zgn_opengl_texture *texture();

 private:
  struct zgn_opengl_texture *texture_;
};

inline struct zgn_opengl_texture *
OpenGLTexture::texture()
{
  return texture_;
}

class OpenGLVertexBuffer : public Buffer
{
 public:
  OpenGLVertexBuffer(App *app, size_t size);
  ~OpenGLVertexBuffer();
  void BufferUpdated();
  inline App *app();
  inline struct zgn_opengl_vertex_buffer *vertex_buffer();

 private:
  App *app_;
  struct zgn_opengl_vertex_buffer *vertex_buffer_;
};

inline App *
OpenGLVertexBuffer::app()
{
  return app_;
}

inline struct zgn_opengl_vertex_buffer *
OpenGLVertexBuffer::vertex_buffer()
{
  return vertex_buffer_;
}

class OpenGLElementArrayBuffer : public Buffer
{
 public:
  OpenGLElementArrayBuffer(App *app, size_t size);
  ~OpenGLElementArrayBuffer();
  void BufferUpdated(enum zgn_opengl_element_array_indices_type type);
  inline App *app();
  inline struct zgn_opengl_element_array_buffer *element_array_buffer();

 private:
  App *app_;
  struct zgn_opengl_element_array_buffer *element_array_buffer_;
};

inline App *
OpenGLElementArrayBuffer::app()
{
  return app_;
}

inline struct zgn_opengl_element_array_buffer *
OpenGLElementArrayBuffer::element_array_buffer()
{
  return element_array_buffer_;
}

class OpenGLShaderProgram
{
 public:
  OpenGLShaderProgram(App *app);
  ~OpenGLShaderProgram();
  void SetUniformVariable(const char *location, glm::mat4 mat);
  void SetUniformVariable(const char *location, glm::vec4 vec);
  void SetUniformVariable(const char *location, glm::vec3 vec);
  bool SetVertexShader(const char *source, size_t len);
  bool SetFragmentShader(const char *source, size_t len);
  void Link();
  inline struct zgn_opengl_shader_program *shader();

 private:
  struct zgn_opengl_shader_program *shader_;
  int vertex_shader_fd_;
  int fragment_shader_fd_;
};

inline struct zgn_opengl_shader_program *
OpenGLShaderProgram::shader()
{
  return shader_;
}

class OpenGLComponent
{
 public:
  OpenGLComponent(App *app, VirtualObject *virtual_object);
  ~OpenGLComponent();
  void Attach(OpenGLVertexBuffer *vertex_buffer);
  void Attach(OpenGLElementArrayBuffer *element_array_buffer);
  void Attach(OpenGLShaderProgram *shader_program);
  void Attach(OpenGLTexture *texture);
  void SetMin(uint32_t min);
  void SetCount(uint32_t count);
  void SetTopology(enum zgn_opengl_topology topology);
  void AddVertexAttribute(uint32_t index, uint32_t size,
      enum zgn_opengl_vertex_attribute_type type, uint32_t normalized,
      uint32_t stride, uint32_t pointer);
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

glm::vec3 glm_vec3_from_wl_array(struct wl_array *array);

void glm_vec3_to_wl_array(glm::vec3 v, struct wl_array *array);

void glm_vec4_to_wl_array(glm::vec4 v, struct wl_array *array);

void glm_mat4_to_wl_array(glm::mat4 m, struct wl_array *array);

glm::quat glm_quat_from_wl_array(struct wl_array *array);

void glm_quat_to_wl_array(glm::quat quaternion, struct wl_array *array);

}  // namespace zukou

#endif  //  ZEN_CLIENT_ZUKOU_H
