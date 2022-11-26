//                                      /
//                                     /
//                 E.-------------------------.H
//                 /|                /       /|
//                / |        ^y     /       / |
//               /  |        |     /       /  |
//              /   |        |    /       /   |
//             /    |        |   /       /    |
//            /     |        |  /       /     |
//           /      |        | /       /      |
//         A.-------------------------.D      |
//  --------|--------------- / -------|------------>x
//          |      F.-------/---------|-------.G
//          |      /       / |        |      /
//          |     /       /  |        |     /
//          |    /       /   |        |    /
//          |   /       /    |        |   /
//          |  /       /     |        |  /
//          | /       /z+    |        | /
//          |/               |        |/
//         B.-------------------------.C
//                           |
//                           |
//                           |

#pragma once

#include <zen-common.h>

#include <array>
#include <cstring>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <vector>

namespace zen::client {

class Application;
class VirtualObject;
class RenderingUnit;
class GlBaseTechnique;
class ShmPool;
class Buffer;
class GlBuffer;
class GlVertexArray;
class GlShader;
class GlProgram;

class Cuboid
{
  struct Vertex {
    float x, y, z;
    float u, v;
  };

 public:
  DISABLE_MOVE_AND_COPY(Cuboid);
  Cuboid() = delete;
  Cuboid(VirtualObject* VirtualObject);
  ~Cuboid();

  bool Render(glm::vec3 half_size, glm::mat4 transform, glm::vec4 color);

  inline size_t vertex_buffer_size();
  inline size_t element_buffer_size();
  inline size_t pool_size();

 private:
  bool Init();

  bool initialized_ = false;

  Application* app_;
  VirtualObject* virtual_object_;
  std::unique_ptr<RenderingUnit> unit_;
  std::unique_ptr<GlBaseTechnique> technique_;
  std::unique_ptr<ShmPool> pool_;
  std::unique_ptr<Buffer> vertex_buffer_;
  std::unique_ptr<Buffer> element_array_buffer_;
  std::unique_ptr<GlBuffer> gl_vertex_buffer_;
  std::unique_ptr<GlBuffer> gl_element_array_buffer_;
  std::unique_ptr<GlVertexArray> vertex_array_;
  std::unique_ptr<GlShader> vertex_shader_;
  std::unique_ptr<GlShader> fragment_shader_;
  std::unique_ptr<GlProgram> program_;

  const Vertex A = {-1, +1, -1, 0, 1};
  const Vertex B = {-1, -1, -1, 0, 0};
  const Vertex C = {+1, -1, -1, 1, 0};
  const Vertex D = {+1, +1, -1, 1, 1};
  const Vertex E = {-1, +1, +1, 1, 0};
  const Vertex F = {-1, -1, +1, 1, 1};
  const Vertex G = {+1, -1, +1, 0, 1};
  const Vertex H = {+1, +1, +1, 0, 0};
  const ushort a = 0, b = 1, c = 2, d = 3, e = 4, f = 5, g = 6, h = 7;

  std::array<Vertex, 8> vertices_ = {A, B, C, D, E, F, G, H};
  std::array<ushort, 24> elements_ = {
      a, b, /**/ b, c, /**/ c, d, /**/ d, a,  //
      e, f, /**/ f, g, /**/ g, h, /**/ h, e,  //
      a, e, /**/ b, f, /**/ c, g, /**/ d, h,  //
  };
};

inline size_t
Cuboid::vertex_buffer_size()
{
  return sizeof(decltype(vertices_)::value_type) * vertices_.size();
}

inline size_t
Cuboid::element_buffer_size()
{
  return sizeof(decltype(elements_)::value_type) * elements_.size();
}

inline size_t
Cuboid::pool_size()
{
  return vertex_buffer_size() + element_buffer_size();
}

}  // namespace zen::client
