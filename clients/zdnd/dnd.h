#ifndef ZEN_CLIENT_ZDND_H
#define ZEN_CLIENT_ZDND_H

#include <zukou.h>

#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "types.h"

#pragma pack(1)
struct StlTriangle {
  float n[3];
  float points[3][3];
  uint16_t unused;
};
#pragma pack()

class ZDnd : public zukou::CuboidWindow
{
 public:
  ZDnd(zukou::App *app, std::string filename, float length);
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

  bool MainLoop();
  void Configure(uint32_t serial, glm::vec3 half_size, glm::quat quaternion);
  void InitGL();
  void UpdateVertex();

  void (*epoll_func)(ZDnd *zdnd);
  int epoll_fd;
  int dnd_fd;

  uint32_t drag_enter_serial;

  struct zgn_data_source *data_source;

  std::vector<StlTriangle> triangle_list;
  std::vector<std::string> mime_type_list;

 private:
  const std::string path_;

  bool running_;

  zukou::OpenGLComponent *component_;
  zukou::OpenGLVertexBuffer *vertex_buffer_;
  zukou::OpenGLShaderProgram *shader_;

  std::vector<Vertex> vertices_;

  struct {
    glm::vec3 origin;
    glm::vec3 direction;
  } ray_;
  bool ray_focus_;

  glm::vec3 min_;
  glm::vec3 max_;
};

#endif  //  ZEN_CLIENT_ZDND_BOX_H

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
