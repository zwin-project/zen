#ifndef ZEN_CLIENT_ZVIEW_INTERNAL_H
#define ZEN_CLIENT_ZVIEW_INTERNAL_H

#include <zukou.h>

#include <fstream>
#include <memory>

class App;

class Reader
{
 public:
  virtual ~Reader(){};
  virtual bool Read([[maybe_unused]] char *buffer, [[maybe_unused]] size_t n)
  {
    return false;
  };
  virtual bool End() { return false; };
  virtual bool GetLine([[maybe_unused]] std::string *str) { return false; };
};

class FileReader : public Reader
{
 public:
  FileReader();
  bool Open(std::string path);
  bool Read(char *buffer, size_t n);
  bool End();
  bool GetLine(std::string *str);

 private:
  std::ifstream ifs_;
};

class HttpReader : public Reader
{
 public:
  HttpReader();
  bool Download(std::string url);
  bool Read(char *buffer, size_t n);
  bool End();
  bool GetLine(std::string *str);

 private:
  std::vector<char> buf_;
  uint32_t cursor_;
};

class Drawable
{
 public:
  Drawable(){};
  virtual ~Drawable(){};
  virtual void Draw([[maybe_unused]] zukou::CuboidWindow *cuboid_window){};
};

class Box : zukou::CuboidWindow
{
 public:
  Box(App *app, glm::vec3 half_size);
  void SetObject(std::unique_ptr<Drawable> object);
  void Configure(uint32_t serial, glm::vec3 half_size, glm::quat quaternion);
  void RayButton(uint32_t serial, uint32_t time, uint32_t button,
      enum zgn_ray_button_state state);
  void DataDeviceDrop();
  void DataDeviceMotion(uint32_t time, glm::vec3 origin, glm::vec3 direction);
  void DataDeviceEnter(uint32_t serial, glm::vec3 origin, glm::vec3 direction,
      struct zgn_data_offer *id);
  void DndFdReady(int fd);

 private:
  App *application_;
  std::unique_ptr<Drawable> object_;
  uint32_t data_device_enter_serial_;
};

class App : public zukou::App
{
 public:
  App();
  ~App();

  bool Show();
  bool UpdateLocation(std::string location);

  inline void set_location(std::string location);

 private:
  std::string location_;
  glm::vec3 half_size_;

  std::shared_ptr<Box> box_;
};

inline void
App::set_location(std::string location)
{
  location_ = location;
}

struct StlVertex {
  glm::vec3 norm;
  glm::vec3 point;
};

class StlObject : public Drawable
{
 public:
  StlObject();
  ~StlObject();
  bool Fill(std::unique_ptr<Reader> reader);
  void Draw(zukou::CuboidWindow *cuboid_window);

 private:
  std::vector<StlVertex> vertices_;
  glm::vec3 max_;
  glm::vec3 min_;
  std::unique_ptr<zukou::OpenGLComponent> component_;
  std::unique_ptr<zukou::OpenGLVertexBuffer> vertex_buffer_;
  std::unique_ptr<zukou::OpenGLShaderProgram> shader_;
};

#endif  //  ZEN_CLIENT_ZVIEW_INTERNAL_H
