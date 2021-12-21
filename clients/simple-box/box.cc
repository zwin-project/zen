#include "box.h"

#include <linux/input-event-codes.h>
#include <string.h>
#include <time.h>
#include <zukou.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <vector>

extern const char *vertex_shader;
extern const char *fragment_shader;
extern const char *texture_fragment_shader;

Box::Box(zukou::App *app, float length)
    : CuboidWindow(app, glm::vec3(length * 1.8))
{
  srand(time(0));
  Vertex points[8];

  length_ = length;
  delta_theta_ = 0;
  delta_phi_ = 0;
  rotate_ = glm::mat4(1.0f);
  blue_ = UINT8_MAX;
  button_.right = false;
  button_.left = false;

  vertex_buffer_ = new zukou::OpenGLVertexBuffer(app, sizeof(Vertex) * 8);

  frame_component_ = new zukou::OpenGLComponent(app, this);
  frame_element_array_ =
      new zukou::OpenGLElementArrayBuffer(app, sizeof(u_short) * 24);
  frame_shader_ = new zukou::OpenGLShaderProgram(app);

  front_component_ = new zukou::OpenGLComponent(app, this);
  front_element_array_ =
      new zukou::OpenGLElementArrayBuffer(app, sizeof(u_short) * 6);
  front_shader_ = new zukou::OpenGLShaderProgram(app);
  texture_ = new zukou::OpenGLTexture(app, 256, 256);

  frame_shader_->SetVertexShader(vertex_shader, strlen(vertex_shader));
  frame_shader_->SetFragmentShader(fragment_shader, strlen(fragment_shader));
  frame_shader_->Link();
  frame_shader_->SetUniformVariable("color", glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
  frame_shader_->SetUniformVariable("rotate", rotate_);

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
  front_shader_->SetUniformVariable("rotate", rotate_);

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
        points[i].p.x = length_ * x;
        points[i].p.y = length_ * y;
        points[i].p.z = length_ * z;
        points[i].u = x < 0 ? 0 : 1;
        points[i].v = y < 0 ? 0 : 1;
        i++;
      }
    }
  }

  {
    u_short frame_indices[24] = {
        0, 1, 2, 3, 4, 5, 6, 7, 0, 4, 1, 5, 2, 6, 3, 7, 0, 2, 1, 3, 4, 6, 5, 7};
    u_short *indices = (u_short *)frame_element_array_->data();
    memcpy(indices, frame_indices, sizeof(frame_indices));
    frame_element_array_->BufferUpdated(
        ZGN_OPENGL_ELEMENT_ARRAY_INDICES_TYPE_UNSIGNED_SHORT);
    frame_component_->Attach(frame_element_array_);
  }

  {
    u_short front_indices[6] = {1, 7, 3, 1, 7, 5};
    u_short *indices = (u_short *)front_element_array_->data();
    memcpy(indices, front_indices, sizeof(front_indices));
    front_element_array_->BufferUpdated(
        ZGN_OPENGL_ELEMENT_ARRAY_INDICES_TYPE_UNSIGNED_SHORT);
    front_component_->Attach(front_element_array_);
  }

  {
    Vertex *vertices = (Vertex *)vertex_buffer_->data();
    memcpy(vertices, points, sizeof(Vertex) * 8);
    vertex_buffer_->BufferUpdated();
    frame_component_->Attach(vertex_buffer_);
    front_component_->Attach(vertex_buffer_);
  }
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

  rotate_ =
      glm::rotate(rotate_, delta_theta_ * 0.001f, glm::vec3(1.0f, 0.0, 0.0f));
  rotate_ =
      glm::rotate(rotate_, delta_phi_ * 0.001f, glm::vec3(0.0f, 1.0, 0.0f));

  frame_shader_->SetUniformVariable("rotate", rotate_);
  front_shader_->SetUniformVariable("rotate", rotate_);

  frame_component_->Attach(frame_shader_);
  front_component_->Attach(front_shader_);

  if (button_.right) this->RotateWithRay();

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
  button_.right = false;
  button_.left = false;
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
Box::RayButton(uint32_t serial, uint32_t time, uint32_t button,
    enum zgn_ray_button_state state)
{
  (void)time;
  switch (button) {
    case BTN_LEFT:
      if (state == ZGN_RAY_BUTTON_STATE_PRESSED) {
        zgn_cuboid_window_move(cuboid_window(), app()->seat(), serial);
        button_.left = true;
      } else {
        button_.left = false;
      }
      break;

    case BTN_RIGHT:
      if (state == ZGN_RAY_BUTTON_STATE_PRESSED) {
        this->RotateWithRay();
        button_.right = true;
      } else {
        button_.right = false;
      }
      break;

    case BTN_MIDDLE:
      this->Rotate(glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)));
      break;
  }
}

void
Box::DrawTexture()
{
  glm::vec2 position, one = {1, 1};
  float distance;
  bool intersected = false;

  Vertex *vertices = (Vertex *)vertex_buffer_->data();

  if (ray_focus_ && glm::intersectRayTriangle(ray_.origin, ray_.direction,
                        glm::vec3(rotate_ * glm::vec4(vertices[1].p, 1)),
                        glm::vec3(rotate_ * glm::vec4(vertices[3].p, 1)),
                        glm::vec3(rotate_ * glm::vec4(vertices[5].p, 1)),
                        position, distance)) {
    intersected = true;
  }

  if (ray_focus_ && !intersected &&
      glm::intersectRayTriangle(ray_.origin, ray_.direction,
          glm::vec3(rotate_ * glm::vec4(vertices[7].p, 1)),
          glm::vec3(rotate_ * glm::vec4(vertices[5].p, 1)),
          glm::vec3(rotate_ * glm::vec4(vertices[3].p, 1)), position,
          distance)) {
    position = one - position;
    intersected = true;
  }

  uint8_t cursor_color = button_.left ? 0 : UINT8_MAX;

  zukou::ColorBGRA *pixel = (zukou::ColorBGRA *)texture_->data();
  for (int x = 0; x < 256; x++) {
    for (int y = 0; y < 256; y++) {
      if (intersected && -8 < position.x * UINT8_MAX - x &&
          position.x * UINT8_MAX - x < 8 && -8 < position.y * UINT8_MAX - y &&
          position.y * UINT8_MAX - y < 8) {
        pixel->a = cursor_color;
        pixel->r = cursor_color;
        pixel->g = cursor_color;
        pixel->b = cursor_color;
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

void
Box::RotateWithRay()
{
  glm::vec3 model_direction = glm::normalize(quaternion() * -ray_.origin);
  glm::vec3 axis = glm::cross(quaternion() * ray_.direction, model_direction);
  float force = M_PI * glm::length(axis) / 30.0f;
  glm::quat delta = glm::angleAxis(force, glm::normalize(axis));
  this->Rotate(delta * this->quaternion());
}

const char *vertex_shader =
    "#version 410\n"
    "uniform mat4 zMVP;\n"
    "uniform mat4 rotate;\n"
    "layout(location = 0) in vec4 position;\n"
    "layout(location = 1) in vec2 v2UVcoordsIn;\n"
    "out vec2 v2UVcoords;\n"
    "void main()\n"
    "{\n"
    "  v2UVcoords = v2UVcoordsIn;\n"
    "  gl_Position = zMVP * rotate * position;\n"
    "}\n";

const char *fragment_shader =
    "#version 410 core\n"
    "uniform vec4 color;\n"
    "out vec4 outputColor;\n"
    "void main()\n"
    "{\n"
    "  outputColor = color;\n"
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
