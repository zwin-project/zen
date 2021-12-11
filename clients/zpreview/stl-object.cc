#include "stl-object.h"

#include <zukou.h>

#include <glm/gtx/string_cast.hpp>
#include <iostream>

extern std::string vertex_shader;
extern std::string fragment_shader;

StlObject::StlObject(
    zukou::App *app, std::vector<StlTriangle> triangles, glm::vec3 half_size)
    : CuboidWindow(app, half_size), min_(FLT_MAX), max_(FLT_MIN)
{
  component_ = new zukou::OpenGLComponent(app_, this);

  vertex_buffer_ = new zukou::OpenGLVertexBuffer(
      app_, sizeof(Vertex) * triangles.size() * 3);

  shader_ = new zukou::OpenGLShaderProgram(app_);

  vertices_.reserve(triangles.size() * 3);

  // we assume stl's z axis as y axis in zigen
  for (auto triangle : triangles) {
    float *n = triangle.n;
    for (int i = 0; i < 3; i++) {
      float *p = triangle.points[i];
      if (p[0] < min_.x) min_.x = p[0];
      if (p[2] < min_.y) min_.y = p[2];
      if (-p[1] < min_.z) min_.z = -p[1];
      if (p[0] > max_.x) max_.x = p[0];
      if (p[2] > max_.y) max_.y = p[2];
      if (-p[1] > max_.z) max_.z = -p[1];
      vertices_.push_back(
          Vertex(glm::vec3(p[0], p[2], -p[1]), glm::vec3(n[0], n[2], -n[1])));
    }
  }

  shader_->SetVertexShader(vertex_shader.c_str(), vertex_shader.size());
  shader_->SetFragmentShader(fragment_shader.c_str(), fragment_shader.size());
  shader_->Link();

  component_->Attach(shader_);
  component_->SetCount(triangles.size() * 6);
  component_->SetTopology(ZGN_OPENGL_TOPOLOGY_TRIANGLES);
  component_->AddVertexAttribute(0, 3, ZGN_OPENGL_VERTEX_ATTRIBUTE_TYPE_FLOAT,
      false, sizeof(Vertex), offsetof(Vertex, point));
  component_->AddVertexAttribute(1, 3, ZGN_OPENGL_VERTEX_ATTRIBUTE_TYPE_FLOAT,
      false, sizeof(Vertex), offsetof(Vertex, norm));
}

StlObject::~StlObject() {}

void
StlObject::Configure(uint32_t serial, glm::vec3 half_size)
{
  zgn_cuboid_window_ack_configure(cuboid_window(), serial);
  half_size_ = half_size;
  glm::vec3 delta(0);

  float zoom = FLT_MAX;
  for (int i = 0; i < 3; i++) {
    delta[i] = (max_[i] + min_[i]) / 2;

    float l = max_[i] - min_[i];
    float r = half_size_[i] * 2 / l;
    if (r < zoom) zoom = r;
  }

  std::cerr << glm::to_string(min_) << std::endl;
  std::cerr << glm::to_string(max_) << std::endl;
  std::cerr << glm::to_string(half_size_) << std::endl;
  std::cerr << zoom << std::endl;

  Vertex *data = (Vertex *)vertex_buffer_->data();

  for (Vertex v : vertices_) {
    data->point = (v.point - delta) * zoom;
    data->norm = v.norm;
    data++;
  }

  vertex_buffer_->BufferUpdated();
  component_->Attach(vertex_buffer_);
  this->Commit();
}

void
StlObject::RayButton(uint32_t serial, uint32_t time, uint32_t button,
    enum zgn_ray_button_state state)
{
  (void)time;
  (void)button;
  if (state == ZGN_RAY_BUTTON_STATE_PRESSED) {
    zgn_cuboid_window_move(cuboid_window(), app()->seat(), serial);
  }
}

std::string vertex_shader(
    "#version 410\n"
    "\n"
    "struct ZLight {\n"
    "  vec4 position;\n"
    "  vec4 diffuse;\n"
    "  vec4 ambient;\n"
    "  vec4 specular;\n"
    "};\n"
    "\n"
    "uniform mat4 zModel;\n"
    "uniform mat4 zVP;\n"
    "uniform ZLight zLight;\n"
    "layout(location = 0) in vec4 localPosition;\n"
    "layout(location = 1) in vec3 norm;\n"
    "out vec4 frontColor;\n"
    "void main()\n"
    "{\n"
    "  vec4 position = zModel * localPosition;\n"
    "  vec3 view = -normalize(position.xyz);\n"
    "  vec3 light = normalize((zLight.position * position.w - "
    "zLight.position.w * position).xyz);\n"
    "  vec3 halfway = normalize(light + view);\n"
    "  float diffuse = max(dot(light, norm), 0.0);\n"
    "  float specular = max(dot(norm, halfway), 0.0);\n"
    "  frontColor = (zLight.diffuse * 0.5 * diffuse + zLight.specular * 0.5 * "
    "specular + zLight.ambient / 10);\n"
    "  gl_Position = zVP * position;\n"
    "}\n");

std::string fragment_shader(
    "#version 410 core\n"
    "out vec4 outputColor;\n"
    "in vec4 frontColor;\n"
    "void main()\n"
    "{\n"
    "  outputColor = vec4(frontColor.xyz, 1.0);\n"
    "}\n");
