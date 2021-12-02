#include "box.h"

#include <string.h>
#include <time.h>
#include <zukou.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
  rx_ = 0;
  ry_ = 0;
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

  Line *edges = (Line *)frame_vertex_buffer_->data();
  for (int i = 0; i < 3; i++) {
    int x = i;
    int y = (i + 1) % 3;
    int z = (i + 2) % 3;
    for (int j = -1; j < 2; j += 2) {
      for (int k = -1; k < 2; k += 2) {
        edges->s.p[x] = -length_;
        edges->s.p[y] = length_ * j;
        edges->s.p[z] = length_ * k;
        edges->e.p[x] = length_;
        edges->e.p[y] = length_ * j;
        edges->e.p[z] = length_ * k;
        edges++;
      }
    }
  }

  Triangle *triangles = (Triangle *)front_vertex_buffer_->data();
  Vertex A = {{-length, -length, length}, 0, 0};
  Vertex B = {{length, -length, length}, 1, 0};
  Vertex C = {{length, length, length}, 1, 1};
  Vertex D = {{-length, length, length}, 0, 1};
  triangles[0].a = A;
  triangles[0].b = C;
  triangles[0].c = B;
  triangles[1].a = A;
  triangles[1].b = C;
  triangles[1].c = D;

  this->DrawTexture();
}

void
Box::Frame(uint32_t time)
{
  (void)time;
  rx_ += (float)(rand() - RAND_MAX / 2) / (float)RAND_MAX;
  rx_ = rx_ > 10 ? 10 : rx_ < -10 ? -10 : rx_;
  ry_ += (float)(rand() - RAND_MAX / 2) / (float)RAND_MAX;
  ry_ = ry_ > 10 ? 10 : ry_ < -10 ? -10 : ry_;

  Vertex *vertex = (Vertex *)frame_vertex_buffer_->data();
  for (int i = 0; i < 24; i++) {
    vertex->p = glm::rotateY(vertex->p, glm::pi<float>() * 0.001f * ry_);
    vertex->p = glm::rotateX(vertex->p, glm::pi<float>() * 0.001f * rx_);
    vertex++;
  }
  frame_vertex_buffer_->BufferUpdated();
  frame_component_->Attach(frame_vertex_buffer_);

  vertex = (Vertex *)front_vertex_buffer_->data();
  for (int i = 0; i < 6; i++) {
    vertex->p = glm::rotateY(vertex->p, glm::pi<float>() * 0.001f * ry_);
    vertex->p = glm::rotateX(vertex->p, glm::pi<float>() * 0.001f * rx_);
    vertex++;
  }
  front_vertex_buffer_->BufferUpdated();
  front_component_->Attach(front_vertex_buffer_);

  this->DrawTexture();

  this->NextFrame();
}

void
Box::RayEnter(uint32_t serial, glm::vec3 origin, glm::vec3 direction)
{
  (void)serial;
  (void)origin;
  (void)direction;
}

void
Box::RayLeave(uint32_t serial)
{
  (void)serial;
}

void
Box::RayMotion(uint32_t time, glm::vec3 origin, glm::vec3 direction)
{
  (void)time;
  (void)origin;
  (void)direction;
  blue_ -= 1;
  if (blue_ == 0) blue_ = UINT8_MAX;
}

void
Box::DrawTexture()
{
  zukou::ColorBGRA *pixel = (zukou::ColorBGRA *)texture_->data();
  for (int x = 0; x < 256; x++) {
    for (int y = 0; y < 256; y++) {
      pixel->a = UINT8_MAX;
      pixel->r = x;
      pixel->g = y;
      pixel->b = blue_;
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
