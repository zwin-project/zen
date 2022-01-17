#define _GNU_SOURCE 1

#include "dnd.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <wayland-client.h>
#include <zukou.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "file-reader.h"
#include "reader.h"
#include "stl-viewer.h"
#include "viewer.h"

static std::string vertex_shader(
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

static std::string fragment_shader(
    "#version 410 core\n"
    "out vec4 outputColor;\n"
    "in vec4 frontColor;\n"
    "mat4 colorize = mat4(1.0, 0.0, 0.0, 0.0,\n"
    "                  0.0, 1.0, 0.0, 0.0,\n"
    "                  0.0, 0.0, 1.0, 0.0,\n"
    "                  0.0, 0.0, 0.0, 1.0);\n"
    "void main()\n"
    "{\n"
    "  outputColor = colorize * vec4(frontColor.xyz, 1.0);\n"
    "}\n");

static const char stl_mime_type[] = "model/stl";

ZDnd::ZDnd(zukou::App *app, std::string filename, float length)
    : CuboidWindow(app, glm::vec3(length * 1.8)),
      path_(filename),
      min_(FLT_MAX),
      max_(FLT_MIN)
{
  uint32_t triangle_count;
  StlTriangle buffer;
  Reader *reader;
  char unused[80];

  this->data_source = nullptr;
  this->mime_type_list = {};

  if (path_.size() == 0) {
    this->triangle_list.reserve(0);
    return;
  }

  FileReader *file_reader = new FileReader();
  if (!file_reader->Open(path_)) return;
  reader = file_reader;
  if (!reader->Read(unused, 80)) return;
  if (std::string(unused, 5) == "solid") {
    std::cerr << path_
              << ": this file seems \"ASCII\" STL which is not "
                 "supported yet"
              << std::endl;
    return;
  }
  if (!reader->Read(
          reinterpret_cast<char *>(&triangle_count), sizeof(triangle_count)))
    return;

  this->triangle_list.reserve(triangle_count);
  for (uint32_t i = 0; i < triangle_count; i++) {
    if (!reader->Read(reinterpret_cast<char *>(&buffer), sizeof(StlTriangle)))
      return;
    this->triangle_list.push_back(buffer);
  }

  delete reader;

  this->InitGL();
}

void
ZDnd::InitGL()
{
  component_ = new zukou::OpenGLComponent(app_, this);

  shader_ = new zukou::OpenGLShaderProgram(app_);

  shader_->SetVertexShader(vertex_shader.c_str(), vertex_shader.size());
  shader_->SetFragmentShader(fragment_shader.c_str(), fragment_shader.size());
  shader_->Link();

  component_->Attach(shader_);
  component_->SetTopology(ZGN_OPENGL_TOPOLOGY_TRIANGLES);
  component_->AddVertexAttribute(0, 3, ZGN_OPENGL_VERTEX_ATTRIBUTE_TYPE_FLOAT,
      false, sizeof(Vertex), offsetof(Vertex, point));
  component_->AddVertexAttribute(1, 3, ZGN_OPENGL_VERTEX_ATTRIBUTE_TYPE_FLOAT,
      false, sizeof(Vertex), offsetof(Vertex, norm));
}

void
epoll_default_func(ZDnd *zdnd)
{
  (void)zdnd;
}

bool
ZDnd::MainLoop()
{
  int ret;
  int epoll_count;
  struct epoll_event ep[16];
  running_ = true;

  this->epoll_fd = epoll_create1(EPOLL_CLOEXEC);
  this->epoll_func = epoll_default_func;

  while (running_) {
    while (wl_display_prepare_read(app()->display()) != 0) {
      if (errno != EAGAIN) return false;
      wl_display_dispatch_pending(app()->display());
    }

    ret = wl_display_flush(app()->display());
    if (ret == -1) return false;

    wl_display_read_events(app()->display());
    wl_display_dispatch_pending(app()->display());

    epoll_count = epoll_wait(this->epoll_fd, ep, 16, 0);
    for (int i = 0; i < epoll_count; i++) {
      this->epoll_func(this);
    }
  }
  return true;
}

void
ZDnd::Configure(uint32_t serial, glm::vec3 half_size, glm::quat quaternion)
{
  (void)quaternion;
  zgn_cuboid_window_ack_configure(cuboid_window(), serial);
  half_size_ = half_size;

  if (triangle_list.size() == 0) return;

  this->UpdateVertex();
}

void
ZDnd::UpdateVertex()
{
  vertex_buffer_ = new zukou::OpenGLVertexBuffer(
      app_, sizeof(Vertex) * this->triangle_list.size() * 3);
  component_->SetCount(this->triangle_list.size() * 6);

  {
    vertices_.clear();
    vertices_.reserve(this->triangle_list.size() * 3);

    // we assume stl's z axis as y axis in zigen
    for (auto triangle : this->triangle_list) {
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
  }

  {
    glm::vec3 delta(0);

    float zoom = FLT_MAX;
    for (int i = 0; i < 3; i++) {
      delta[i] = (max_[i] + min_[i]) / 2;

      float l = max_[i] - min_[i];
      float r = half_size_[i] * 2 / l;
      if (r < zoom) zoom = r;
    }

    Vertex *data = (Vertex *)vertex_buffer_->data();

    for (Vertex v : vertices_) {
      data->point = (v.point - delta) * zoom;
      data->norm = v.norm;
      data++;
    }
  }

  vertex_buffer_->BufferUpdated();
  component_->Attach(vertex_buffer_);
  this->Commit();
}

void
ZDnd::RayEnter(uint32_t serial, glm::vec3 origin, glm::vec3 direction)
{
  (void)serial;
  ray_focus_ = true;
  ray_.origin = origin;
  ray_.direction = direction;
}

void
ZDnd::RayLeave(uint32_t serial)
{
  (void)serial;
  ray_focus_ = false;
}

void
ZDnd::RayMotion(uint32_t time, glm::vec3 origin, glm::vec3 direction)
{
  (void)time;
  ray_.origin = origin;
  ray_.direction = direction;
}

void
ZDnd::DataOfferOffer(const char *mime_type)
{
  this->mime_type_list.push_back((mime_type));
}

void
ZDnd::DataOfferSourceActions(uint32_t source_actions)
{
  (void)source_actions;
}

void
ZDnd::DataOfferAction(uint32_t dnd_action)
{
  (void)dnd_action;
}

void
ZDnd::DataDeviceEnter(uint32_t serial, glm::vec3 origin, glm::vec3 direction,
    struct zgn_data_offer *id)
{
  (void)origin;
  (void)direction;
  (void)id;
  this->drag_enter_serial = serial;

  for (std::string type : this->mime_type_list)
    zgn_data_offer_accept(app()->data_offer(), serial, type.c_str());
}

void
ZDnd::DataDeviceLeave()
{
  if (app()->data_offer()) zgn_data_offer_destroy(app()->data_offer());
}

void
ZDnd::DataDeviceMotion(uint32_t time, glm::vec3 origin, glm::vec3 direction)
{
  (void)time;
  (void)origin;
  (void)direction;

  for (std::string type : this->mime_type_list)
    zgn_data_offer_accept(
        app()->data_offer(), this->drag_enter_serial, type.c_str());
}

static void
epoll_io_func(ZDnd *zdnd)
{
  StlTriangle buf;
  std::vector<StlTriangle> tmp = {};

  while (read(zdnd->dnd_fd, reinterpret_cast<char *>(&buf),
             sizeof(StlTriangle)) > 0)
    tmp.push_back(buf);
  close(zdnd->dnd_fd);

  if (tmp.size() == 0) return;

  // FIXME: send finish

  if (zdnd->triangle_list.size() == 0) {
    zdnd->InitGL();
  }
  zdnd->triangle_list.swap(tmp);
  zdnd->UpdateVertex();
}

void
ZDnd::DataDeviceDrop()
{
  int pipefd[2];
  struct epoll_event ep;

  if (pipe2(pipefd, O_CLOEXEC) == -1) return;

  if (this->data_source != nullptr) {
    zgn_data_offer_finish(app()->data_offer());
    return;
  }

  zgn_data_offer_receive(app()->data_offer(), stl_mime_type, pipefd[1]);
  close(pipefd[1]);

  this->epoll_func = epoll_io_func;
  this->dnd_fd = pipefd[0];
  ep.events = EPOLLIN;

  epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, this->dnd_fd, &ep);
}

void
data_source_target(
    void *data, struct zgn_data_source *data_source, const char *mime_type)
{
  (void)data;
  (void)data_source;
  (void)mime_type;
}

void
data_source_send(void *data, struct zgn_data_source *data_source,
    const char *mime_type, int32_t fd)
{
  (void)data_source;
  (void)mime_type;

  ZDnd *zdnd = (ZDnd *)data;
  unsigned int size = zdnd->triangle_list.size();
  StlTriangle buf;

  for (int i = 0; i < (int)size; i++) {
    buf = zdnd->triangle_list[i];
    if (write(fd, &buf, sizeof(StlTriangle)) < 0) abort();
  }
  close(fd);
}

void
data_source_cancelled(void *data, struct zgn_data_source *data_source)
{
  (void)data_source;

  ZDnd *zdnd = (ZDnd *)data;
  if (zdnd->data_source) zgn_data_source_destroy(zdnd->data_source);

  zdnd->mime_type_list.clear();
}

void
data_source_dnd_drop_performed(void *data, struct zgn_data_source *data_source)
{
  (void)data;
  (void)data_source;
}

void
data_source_dnd_finished(void *data, struct zgn_data_source *data_source)
{
  (void)data;
  (void)data_source;

  ZDnd *zdnd = (ZDnd *)data;
  if (zdnd->data_source) zgn_data_source_destroy(zdnd->data_source);

  zdnd->mime_type_list.clear();
}

void
data_source_action(
    void *data, struct zgn_data_source *data_source, uint32_t dnd_action)
{
  (void)data;
  (void)data_source;
  (void)dnd_action;
}

static const struct zgn_data_source_listener data_source_listener = {
    data_source_target,
    data_source_send,
    data_source_cancelled,
    data_source_dnd_drop_performed,
    data_source_dnd_finished,
    data_source_action,
};

void
ZDnd::RayButton(uint32_t serial, uint32_t time, uint32_t button,
    enum zgn_ray_button_state state)
{
  (void)time;
  (void)button;

  if (state == ZGN_RAY_BUTTON_STATE_PRESSED) {
    this->data_source = zgn_data_device_manager_create_data_source(
        app()->data_device_manager());
    zgn_data_source_add_listener(
        this->data_source, &data_source_listener, this);
    zgn_data_source_offer(this->data_source, stl_mime_type);

    zgn_data_device_start_drag(app()->data_device(), this->data_source,
        this->virtual_object(), NULL, serial);
  }
}
