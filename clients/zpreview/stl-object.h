#ifndef ZEN_CLIENT_ZPREVIEW_STL_OBJECT_H
#define ZEN_CLIENT_ZPREVIEW_STL_OBJECT_H

#include <zukou.h>

#include <glm/glm.hpp>
#include <vector>

#pragma pack(1)
struct StlTriangle {
  float n[3];
  float points[3][3];
  uint16_t unused;
};
#pragma pack()

struct Vertex {
  Vertex(glm::vec3 point, glm::vec3 norm)
  {
    this->point = point;
    this->norm = norm;
  };

  glm::vec3 point;
  glm::vec3 norm;
};

class StlObject : public zukou::CuboidWindow
{
 public:
  StlObject(
      zukou::App *app, std::vector<StlTriangle> triangles, glm::vec3 half_size);
  ~StlObject();
  void Configure(uint32_t serial, glm::vec3 half_size);
  virtual void RayButton(uint32_t serial, uint32_t time, uint32_t button,
      enum zgn_ray_button_state state);

 private:
  zukou::OpenGLComponent *component_;
  zukou::OpenGLVertexBuffer *vertex_buffer_;
  zukou::OpenGLShaderProgram *shader_;

  std::vector<Vertex> vertices_;

  glm::vec3 min_;
  glm::vec3 max_;
};

#endif  //  ZEN_CLIENT_ZPREVIEW_STL_OBJECT_H
