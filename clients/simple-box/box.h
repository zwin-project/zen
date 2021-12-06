#ifndef ZEN_CLIENT_SIMPLE_BOX_BOX_H
#define ZEN_CLIENT_SIMPLE_BOX_BOX_H

#include <zukou.h>

#include <glm/glm.hpp>

struct Vertex {
  glm::vec3 p;
  float u, v;
};

class Box : public zukou::CuboidWindow
{
 public:
  Box(zukou::App *app, float length);
  void Frame(uint32_t time);
  virtual void RayEnter(uint32_t serial, glm::vec3 origin, glm::vec3 direction);
  virtual void RayLeave(uint32_t serial);
  virtual void RayMotion(uint32_t time, glm::vec3 origin, glm::vec3 direction);
  virtual void RayButton(uint32_t serial, uint32_t time, uint32_t button,
      enum zgn_ray_button_state state);
  void DrawTexture();

 private:
  zukou::OpenGLVertexBuffer *vertex_buffer_;

  zukou::OpenGLComponent *frame_component_;
  zukou::OpenGLElementArrayBuffer *frame_element_array_;
  zukou::OpenGLShaderProgram *frame_shader_;

  zukou::OpenGLComponent *front_component_;
  zukou::OpenGLElementArrayBuffer *front_element_array_;
  zukou::OpenGLShaderProgram *front_shader_;
  zukou::OpenGLTexture *texture_;

  float length_;
  float theta_;
  float phi_;
  float delta_theta_;
  float delta_phi_;
  Vertex points_[8];

  uint8_t blue_;

  struct {
    glm::vec3 origin;
    glm::vec3 direction;
  } ray_;
  bool ray_focus_;

  bool button_pressed_;
};

#endif  //  ZEN_CLIENT_SIMPLE_BOX_BOX_H

// Box.points_
//                                         /
//                                        /
//                    2.-------------------------.6
//                    /|                /       /|
//                   / |        ^y     /       / |
//                  /  |        |     /       /  |
//                 /   |        |    /       /   |
//                /    |        |   /       /    |
//               /     |        |  /       /     |
//              /      |        | /       /      |
//            3.-------------------------.7      |
//  -----------|--------------- / -------|------------>x
//             |      0.-------/---------|-------.4
//             |      /       / |        |      /
//             |     /       /  |        |     /
//             |    /       /   |        |    /
//             |   /       /    |        |   /
//             |  /       /     |        |  /
//             | /       /z+    |        | /
//             |/               |        |/
//            1.-------------------------.5
//                              |
//                              |
//                              |
