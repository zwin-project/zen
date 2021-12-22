#include "obj-object.h"

#include <zukou.h>

#include <cstring>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

static std::string vertex_shader(
    "#version 410\n"
    "\n"
    "uniform mat4 model;\n"
    "uniform mat4 zModel;\n"
    "uniform mat4 zVP;\n"
    "layout(location = 0) in vec4 localPosition;\n"
    "layout(location = 1) in vec3 normUnnormalized;\n"
    "varying vec4 position;\n"
    "varying vec3 norm;\n"
    "void main()\n"
    "{\n"
    "  norm = normalize(model * vec4(normUnnormalized, 0)).xyz;\n"
    "  position = zModel * model * localPosition;\n"
    "  gl_Position = zVP * position;\n"
    "}\n");

static std::string fragment_shader(
    "#version 410 core\n"
    "\n"
    "struct ZLight {\n"
    "  vec4 position;\n"
    "  vec4 diffuse;\n"
    "  vec4 ambient;\n"
    "  vec4 specular;\n"
    "};\n"
    "\n"
    "uniform ZLight zLight;\n"
    "out vec4 outputColor;\n"
    "varying vec4 position;\n"
    "varying vec3 norm;\n"
    "void main()\n"
    "{\n"
    "  vec3 view = -normalize(position.xyz);\n"
    "  vec3 light = normalize((zLight.position -  position).xyz);\n"
    "  vec3 halfway = normalize(light + view);\n"
    "  vec3 fnorm = normalize(norm);\n"
    "  float diffuse = max(dot(light, fnorm), 0.0);\n"
    "  float specular = pow(max(dot(fnorm, halfway), 0.0), 50);\n"
    "  outputColor = vec4((zLight.diffuse * 0.1 * diffuse + zLight.specular * "
    "0.5 * specular + zLight.ambient).xyz, 1.0);\n"
    "}\n");

ObjObject::ObjObject(zukou::App *app,
    std::vector<std::vector<ObjFacePoint>> *faces,
    std::vector<glm::vec3> *vertices, std::vector<glm::vec3> *norms,
    glm::vec3 half_size)
    : CuboidWindow(app, half_size), min_(FLT_MAX), max_(FLT_MIN)
{
  std::vector<Vertex> buffer;
  buffer.reserve(faces->size() * 3);

  for (std::vector<ObjFacePoint> face : *faces) {
    for (size_t i = 2; i < face.size(); i++) {
      ObjFacePoint face_point = face[0];
      buffer.push_back(Vertex(vertices->at(face_point.vertex_index),
          norms->at(face_point.norm_index)));
      face_point = face[i - 1];
      buffer.push_back(Vertex(vertices->at(face_point.vertex_index),
          norms->at(face_point.norm_index)));
      face_point = face[i];
      buffer.push_back(Vertex(vertices->at(face_point.vertex_index),
          norms->at(face_point.norm_index)));
    }

    for (auto face_point : face) {
      glm::vec3 v = vertices->at(face_point.vertex_index);
      if (v.x < min_.x) min_.x = v.x;
      if (v.y < min_.y) min_.y = v.y;
      if (v.z < min_.z) min_.z = v.z;
      if (v.x > max_.x) max_.x = v.x;
      if (v.y > max_.y) max_.y = v.y;
      if (v.z > max_.z) max_.z = v.z;
    }
  }

  component_ = new zukou::OpenGLComponent(app_, this);

  vertex_buffer_ =
      new zukou::OpenGLVertexBuffer(app_, sizeof(Vertex) * buffer.size());

  shader_ = new zukou::OpenGLShaderProgram(app_);

  shader_->SetVertexShader(vertex_shader.c_str(), vertex_shader.size());
  shader_->SetFragmentShader(fragment_shader.c_str(), fragment_shader.size());
  shader_->Link();

  component_->Attach(shader_);
  component_->SetCount(buffer.size());
  component_->SetTopology(ZGN_OPENGL_TOPOLOGY_TRIANGLES);
  component_->AddVertexAttribute(0, 3, ZGN_OPENGL_VERTEX_ATTRIBUTE_TYPE_FLOAT,
      false, sizeof(Vertex), offsetof(Vertex, point));
  component_->AddVertexAttribute(1, 3, ZGN_OPENGL_VERTEX_ATTRIBUTE_TYPE_FLOAT,
      false, sizeof(Vertex), offsetof(Vertex, norm));

  {
    Vertex *data = (Vertex *)vertex_buffer_->data();
    std::memcpy(data, buffer.data(), sizeof(Vertex) * buffer.size());

    vertex_buffer_->BufferUpdated();
    component_->Attach(vertex_buffer_);
  }
}

ObjObject::~ObjObject()
{
  delete component_;
  delete vertex_buffer_;
  delete shader_;
}

void
ObjObject::Configure(uint32_t serial, glm::vec3 half_size, glm::quat quaternion)
{
  zgn_cuboid_window_ack_configure(cuboid_window(), serial);
  half_size_ = half_size;
  quaternion_ = quaternion;
  glm::mat4 model = glm::mat4(1);
  glm::vec3 delta(0);

  float zoom = FLT_MAX;
  for (int i = 0; i < 3; i++) {
    delta[i] = (max_[i] + min_[i]) / 2;

    float l = max_[i] - min_[i];
    float r = half_size_[i] * 2 / l;
    if (r < zoom) zoom = r;
  }

  model = glm::rotate(model, -(float)M_PI / 2, glm::vec3(1.0, 0.0, 0.0));
  model = glm::rotate(model, -(float)M_PI / 2, glm::vec3(0.0, 0.0, 1.0));
  model = glm::scale(model, glm::vec3(zoom));
  model = glm::translate(model, -delta);

  shader_->SetUniformVariable("model", model);
  component_->Attach(shader_);

  this->Commit();
}

void
ObjObject::RayButton(uint32_t serial, uint32_t time, uint32_t button,
    enum zgn_ray_button_state state)
{
  (void)time;
  (void)button;
  if (state == ZGN_RAY_BUTTON_STATE_PRESSED) {
    zgn_cuboid_window_move(cuboid_window(), app()->seat(), serial);
  }
}
