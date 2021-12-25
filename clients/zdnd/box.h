#ifndef ZEN_CLIENT_SIMPLE_BOX_BOX_H
#define ZEN_CLIENT_SIMPLE_BOX_BOX_H

#include <zukou.h>

#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "stl-object.h"
#include "types.h"

class Box : public zukou::CuboidWindow
{
 public:
  Box(zukou::App *app, std::string filename, float length);
  void Frame(uint32_t time);
  void Show(std::vector<StlTriangle> triangles);
  virtual void DataOfferOffer(const char *mime_type);
  virtual void DataOfferSourceActions(uint32_t source_actions);
  virtual void DataOfferAction(uint32_t dnd_action);
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
  void DrawTexture();
  void Configure(uint32_t serial, glm::vec3 half_size);

  struct wl_array mime_type_list;
  struct zgn_data_source *data_source;

 private:
  const std::string path_;

  zukou::OpenGLComponent *component_;
  zukou::OpenGLVertexBuffer *vertex_buffer_;
  zukou::OpenGLShaderProgram *shader_;

  uint32_t triangle_count_;

  std::vector<Vertex> vertices_;

  struct {
    glm::vec3 origin;
    glm::vec3 direction;
  } ray_;
  bool ray_focus_;

  uint32_t drag_enter_serial_;

  glm::vec3 min_;
  glm::vec3 max_;
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
