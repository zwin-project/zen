#include "box.h"

#include <string.h>
#include <time.h>
#include <zukou.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <vector>

extern const char *vertex_shader;
extern const char *green_fragment_shader;
extern const char *texture_fragment_shader;

struct Vertex {
  glm::vec3 p;
  float u, v;
};

struct Line {
  Vertex s, e;
};

struct Triangle {
  Vertex a, b, c;
};

Box::Box(zukou::App *app, float length)
    : CuboidWindow(app, glm::vec3(length * 1.8))
{
  srand(time(0));

  length_ = length;
  theta_ = 0;
  phi_ = 0;
  delta_theta_ = 0;
  delta_phi_ = 0;
  blue_ = UINT8_MAX;

  frame_component_ = new zukou::OpenGLComponent(app, this);
  frame_vertex_buffer_ = new zukou::OpenGLVertexBuffer(app, sizeof(Line) * 12);
  frame_shader_ = new zukou::OpenGLShaderProgram(app);

  front_component_ = new zukou::OpenGLComponent(app, this);
  front_vertex_buffer_ =
      new zukou::OpenGLVertexBuffer(app, sizeof(Triangle) * 2);
  front_shader_ = new zukou::OpenGLShaderProgram(app);
  texture_ = new zukou::OpenGLTexture(app, 256, 256);

  frame_shader_->SetVertexShader(vertex_shader, strlen(vertex_shader));
  frame_shader_->SetFragmentShader(
      green_fragment_shader, strlen(green_fragment_shader));
  frame_shader_->Link();

  frame_component_->Attach(frame_shader_);
  frame_component_->SetCount(24);
  frame_component_->SetTopology(ZGN_OPENGL_TOPOLOGY_LINES);
  frame_component_->AddVertexAttribute(0, 3,
      ZGN_OPENGL_VERTEX_ATTRIBUTE_TYPE_FLOAT, false, sizeof(Vertex),
      offsetof(Vertex, p));

  front_shader_->SetVertexShader(vertex_shader, strlen(vertex_shader));
  front_shader_->SetFragmentShader(
      texture_fragment_shader, strlen(texture_fragment_shader));
  front_shader_->Link();

  front_component_->Attach(front_shader_);
  front_component_->SetCount(6);
  front_component_->SetTopology(ZGN_OPENGL_TOPOLOGY_TRIANGLES);
  front_component_->AddVertexAttribute(0, 3,
      ZGN_OPENGL_VERTEX_ATTRIBUTE_TYPE_FLOAT, false, sizeof(Vertex),
      offsetof(Vertex, p));
  front_component_->AddVertexAttribute(1, 2,
      ZGN_OPENGL_VERTEX_ATTRIBUTE_TYPE_FLOAT, false, sizeof(Vertex),
      offsetof(Vertex, u));

  int i = 0;
  for (int x = -1; x < 2; x += 2) {
    for (int y = -1; y < 2; y += 2) {
      for (int z = -1; z < 2; z += 2) {
        points_[i].x = length_ * x;
        points_[i].y = length_ * y;
        points_[i].z = length_ * z;
        i++;
      }
    }
  }

  for (int i = 0; i < 8; i++) {
    rotated_points_[i] = glm::rotateY(points_[i], theta_);
    rotated_points_[i] = glm::rotateX(rotated_points_[i], phi_);
  }

  this->DrawFrame();

  this->DrawFront();

  this->DrawTexture();
}

void
Box::Frame(uint32_t time)
{
  (void)time;
  delta_theta_ += (float)(rand() - RAND_MAX / 2) / (float)RAND_MAX;
  delta_theta_ = delta_theta_ > 10    ? 10
                 : delta_theta_ < -10 ? -10
                                      : delta_theta_;
  delta_phi_ += (float)(rand() - RAND_MAX / 2) / (float)RAND_MAX;
  delta_phi_ = delta_phi_ > 10 ? 10 : delta_phi_ < -10 ? -10 : delta_phi_;

  theta_ += glm::pi<float>() * 0.001f * delta_theta_;
  phi_ += glm::pi<float>() * 0.001f * delta_phi_;

  for (int i = 0; i < 8; i++) {
    rotated_points_[i] = glm::rotateY(points_[i], theta_);
    rotated_points_[i] = glm::rotateX(rotated_points_[i], phi_);
  }

  this->DrawFrame();

  this->DrawFront();

  this->DrawTexture();

  this->NextFrame();
}

void
Box::RayEnter(uint32_t serial, glm::vec3 origin, glm::vec3 direction)
{
  (void)serial;
  ray_focus_ = true;
  ray_.origin = origin;
  ray_.direction = direction;
}

void
Box::RayLeave(uint32_t serial)
{
  (void)serial;
  ray_focus_ = false;
}

void
Box::RayMotion(uint32_t time, glm::vec3 origin, glm::vec3 direction)
{
  (void)time;
  ray_.origin = origin;
  ray_.direction = direction;
  blue_ -= 1;
  if (blue_ == 0) blue_ = UINT8_MAX;
}

void
Box::DrawFrame()
{
  Vertex *vertex = (Vertex *)frame_vertex_buffer_->data();
  int indices[24] = {
      0, 1, 2, 3, 4, 5, 6, 7, 0, 4, 1, 5, 2, 6, 3, 7, 0, 2, 1, 3, 4, 6, 5, 7};

  for (int i = 0; i < 24; i++) vertex[i].p = rotated_points_[indices[i]];

  frame_vertex_buffer_->BufferUpdated();
  frame_component_->Attach(frame_vertex_buffer_);
}

void
Box::DrawFront()
{
  Vertex *vertex = (Vertex *)front_vertex_buffer_->data();
  int indices[6] = {1, 7, 3, 1, 7, 5};
  int uvs[6][2] = {{0, 0}, {1, 1}, {0, 1}, {0, 0}, {1, 1}, {1, 0}};

  for (int i = 0; i < 6; i++) {
    vertex[i].p = rotated_points_[indices[i]];
    vertex[i].u = uvs[i][0];
    vertex[i].v = uvs[i][1];
  }

  front_vertex_buffer_->BufferUpdated();
  front_component_->Attach(front_vertex_buffer_);
}

void
Box::DrawTexture()
{
  glm::vec2 position, one = {1, 1};
  float distance;
  bool intersected = false;

  if (ray_focus_ &&
      glm::intersectRayTriangle(ray_.origin, ray_.direction, rotated_points_[1],
          rotated_points_[3], rotated_points_[5], position, distance)) {
    intersected = true;
  }

  if (ray_focus_ && !intersected &&
      glm::intersectRayTriangle(ray_.origin, ray_.direction, rotated_points_[7],
          rotated_points_[5], rotated_points_[3], position, distance)) {
    position = one - position;
    intersected = true;
  }

  zukou::ColorBGRA *pixel = (zukou::ColorBGRA *)texture_->data();
  for (int x = 0; x < 256; x++) {
    for (int y = 0; y < 256; y++) {
      if (intersected && -8 < position.x * UINT8_MAX - x &&
          position.x * UINT8_MAX - x < 8 && -8 < position.y * UINT8_MAX - y &&
          position.y * UINT8_MAX - y < 8) {
        pixel->a = UINT8_MAX;
        pixel->r = UINT8_MAX;
        pixel->g = UINT8_MAX;
        pixel->b = UINT8_MAX;
      } else {
        pixel->a = UINT8_MAX;
        pixel->r = x;
        pixel->g = y;
        pixel->b = blue_;
      }
      pixel++;
    }
  }

  texture_->BufferUpdated();
  front_component_->Attach(texture_);
}

const char *vertex_shader =
    "#version 410\n"
    "uniform mat4 mvp;\n"
    "layout(location = 0) in vec4 position;\n"
    "layout(location = 1) in vec2 v2UVcoordsIn;\n"
    "out vec2 v2UVcoords;\n"
    "void main()\n"
    "{\n"
    "  v2UVcoords = v2UVcoordsIn;\n"
    "  gl_Position = mvp * position;\n"
    "}\n";

const char *green_fragment_shader =
    "#version 410 core\n"
    "out vec4 outputColor;\n"
    "void main()\n"
    "{\n"
    "  outputColor = vec4(0.0, 1.0, 0.0, 0.5);\n"
    "}\n";

const char *texture_fragment_shader =
    "#version 410 core\n"
    "uniform sampler2D userTexture;\n"
    "in vec2 v2UVcoords;\n"
    "out vec4 outputColor;\n"
    "void main()\n"
    "{\n"
    "  outputColor = texture(userTexture, v2UVcoords);\n"
    "}\n";
