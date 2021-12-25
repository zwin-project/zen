#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-result"

#define _GNU_SOURCE 1

#include "box.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
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

static const char sample_mime_type[] = "text/plain;charset=utf-8";

Box::Box(zukou::App *app, std::string filename, float length)
    : CuboidWindow(app, glm::vec3(length * 1.8)),
      path_(filename),
      min_(FLT_MAX),
      max_(FLT_MIN)
{
  if (path_.size() > 0) {
    StlTriangle buffer;
    Reader *reader;
    char unused[80];

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

    if (!reader->Read(reinterpret_cast<char *>(&triangle_count_),
            sizeof(triangle_count_)))
      return;

    app->triangle_list.reserve(triangle_count_);

    for (uint32_t i = 0; i < triangle_count_; i++) {
      if (!reader->Read(reinterpret_cast<char *>(&buffer), sizeof(StlTriangle)))
        return;
      app->triangle_list.push_back(buffer);
    }

    delete reader;
  } else {
    triangle_count_ = 0;
    app->triangle_list.reserve(0);
  }

  component_ = new zukou::OpenGLComponent(app, this);

  shader_ = new zukou::OpenGLShaderProgram(app);

  shader_->SetVertexShader(vertex_shader.c_str(), vertex_shader.size());
  shader_->SetFragmentShader(fragment_shader.c_str(), fragment_shader.size());
  shader_->Link();

  component_->Attach(shader_);
  component_->SetTopology(ZGN_OPENGL_TOPOLOGY_TRIANGLES);
  component_->AddVertexAttribute(0, 3, ZGN_OPENGL_VERTEX_ATTRIBUTE_TYPE_FLOAT,
      false, sizeof(Vertex), offsetof(Vertex, point));
  component_->AddVertexAttribute(1, 3, ZGN_OPENGL_VERTEX_ATTRIBUTE_TYPE_FLOAT,
      false, sizeof(Vertex), offsetof(Vertex, norm));

  this->Show(app->triangle_list);
}

void
Box::Frame(uint32_t time)
{
  (void)time;

  if (app()->triangle_list.size() != triangle_count_) {
    triangle_count_ = app()->triangle_list.size();
    printf("new triangle size: %d!!!!!\n", triangle_count_);
    this->Show(app()->triangle_list);
    printf("repaint!!!!!!!\n");
  }

  this->NextFrame();
}

void
Box::Show(std::vector<StlTriangle> triangles)
{
  if (triangle_count_ == 0) return;
  printf("box::show!!!\n");

  vertex_buffer_ = new zukou::OpenGLVertexBuffer(
      app_, sizeof(Vertex) * triangles.size() * 3);
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

  component_->SetCount(triangles.size() * 6);

  glm::vec3 delta(0);

  if (triangle_count_ == 0) return;

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

  vertex_buffer_->BufferUpdated();
  component_->Attach(vertex_buffer_);
  this->Commit();
}

void
Box::Configure(uint32_t serial, glm::vec3 half_size)
{
  zgn_cuboid_window_ack_configure(cuboid_window(), serial);
  half_size_ = half_size;
  glm::vec3 delta(0);

  if (triangle_count_ == 0) return;

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

  vertex_buffer_->BufferUpdated();
  component_->Attach(vertex_buffer_);
  this->Commit();
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
}

void
Box::DataOfferOffer(const char *mime_type)
{
  char **type;

  type = (char **)wl_array_add(&this->mime_type_list, sizeof *type);
  *type = strdup(mime_type);
}

void
Box::DataOfferSourceActions(uint32_t source_actions)
{}

void
Box::DataOfferAction(uint32_t dnd_action)
{}

void
Box::DataDeviceEnter(uint32_t serial, glm::vec3 origin, glm::vec3 direction,
    struct zgn_data_offer *id)
{
  char **type;

  this->drag_enter_serial_ = serial;

  // TODO: refactor
  for (type = (char **)(&this->mime_type_list)->data;
       (const char *)type < ((const char *)(&this->mime_type_list)->data +
                                (&this->mime_type_list)->size);
       (type)++)
    zgn_data_offer_accept(app()->data_offer(), serial, *type);
}

void
Box::DataDeviceLeave()
{
  // data_offerのdestroy
  // if (app()->data_offer()) zgn_data_offer_destroy(app()->data_offer());
}

void
Box::DataDeviceMotion(uint32_t time, glm::vec3 origin, glm::vec3 direction)
{
  char **type;

  // TODO: refactor
  for (type = (char **)(&this->mime_type_list)->data;
       (const char *)type < ((const char *)(&this->mime_type_list)->data +
                                (&this->mime_type_list)->size);
       (type)++)
    zgn_data_offer_accept(app()->data_offer(), this->drag_enter_serial_, *type);
}

static void
io_func(zukou::App *app)
{
  StlTriangle buf;

  app->triangle_list.clear();
  while (
      read(app->fd, reinterpret_cast<char *>(&buf), sizeof(StlTriangle)) > 0) {
    app->triangle_list.push_back(buf);
  }
  printf("triangle size: %ld!!!!!\n", app->triangle_list.size());
  for (int i = 0; i < (int)10; i++) {
    buf = app->triangle_list[i];
    printf("(%f, %f, %f)\n", buf.n[0], buf.n[1], buf.n[2]);
  }
  close(app->fd);

  zgn_data_offer_finish(app->data_offer());
  zgn_data_offer_destroy(app->data_offer());
}

void
Box::DataDeviceDrop()
{
  int pipefd[2];

  if (pipe2(pipefd, O_CLOEXEC) == -1) return;

  zgn_data_offer_receive(app()->data_offer(), sample_mime_type, pipefd[1]);
  close(pipefd[1]);

  struct epoll_event ep;

  app()->epoll_func = io_func;
  app()->fd = pipefd[0];
  ep.events = EPOLLIN;
  ep.data.ptr = app();

  epoll_ctl(app()->epoll_fd, EPOLL_CTL_ADD, app()->fd, &ep);
}

void
data_source_target(
    void *data, struct zgn_data_source *data_source, const char *mime_type)
{}

void
data_source_send(void *data, struct zgn_data_source *data_source,
    const char *mime_type, int32_t fd)
{
  Box *box = (Box *)data;
  unsigned int size = box->app()->triangle_list.size();
  StlTriangle buf;

  for (int i = 0; i < (int)size; i++) {
    buf = box->app()->triangle_list[i];
    if (write(fd, &buf, sizeof(StlTriangle)) < 0) abort();
  }
  printf("triangle size: %d!!!!!\n", size);
  for (int i = 0; i < (int)10; i++) {
    buf = box->app()->triangle_list[i];
    printf("(%f, %f, %f)\n", buf.n[0], buf.n[1], buf.n[2]);
  }

  close(fd);
}

void
data_source_cancelled(void *data, struct zgn_data_source *data_source)
{
  // destroy
}

void
data_source_dnd_drop_performed(void *data, struct zgn_data_source *data_source)
{}

void
data_source_dnd_finished(void *data, struct zgn_data_source *data_source)
{
  Box *box = (Box *)data;
  if (box->data_source) zgn_data_source_destroy(box->data_source);
}

void
data_source_action(
    void *data, struct zgn_data_source *data_source, uint32_t dnd_action)
{}

static const struct zgn_data_source_listener data_source_listener = {
    data_source_target,
    data_source_send,
    data_source_cancelled,
    data_source_dnd_drop_performed,
    data_source_dnd_finished,
    data_source_action,
};

void
Box::RayButton(uint32_t serial, uint32_t time, uint32_t button,
    enum zgn_ray_button_state state)
{
  (void)time;
  (void)button;

  // std::random_device rd;
  // std::default_random_engine engine(rd);
  // std::uniform_real_distribution<float> distr(0, 1.0);

  if (state == ZGN_RAY_BUTTON_STATE_PRESSED) {
    // sprintf(buf,
    //     "#version 410 core\n"
    //     "out vec4 outputColor;\n"
    //     "in vec4 frontColor;\n"
    //     "mat4 colorize = mat4( %f, 0.0, 0.0, 0.0,\n"
    //     "                     0.0,  %f, 0.0, 0.0,\n"
    //     "                     0.0, 0.0,  %f, 0.0,\n"
    //     "                     0.0, 0.0, 0.0, 1.0);\n"
    //     "void main()\n"
    //     "{\n"
    //     "  outputColor = colorize * vec4(frontColor.xyz, 1.0);\n"
    //     "}\n",
    //     (float)std::rand() * 1.0 / RAND_MAX,
    //     (float)std::rand() * 1.0 / RAND_MAX,
    //     (float)std::rand() * 1.0 / RAND_MAX);
    // fragment_shader = std::string(buf);
    // shader_->SetVertexShader(vertex_shader.c_str(), vertex_shader.size());
    // shader_->SetFragmentShader(fragment_shader.c_str(),
    // fragment_shader.size()); shader_->Link(); component_->Attach(shader_);
    // this->Commit();

    this->data_source = zgn_data_device_manager_create_data_source(
        app()->data_device_manager());
    zgn_data_source_add_listener(
        this->data_source, &data_source_listener, this);
    zgn_data_source_offer(this->data_source, sample_mime_type);

    zgn_data_device_start_drag(app()->data_device(), this->data_source,
        this->virtual_object(), NULL, serial);

    wl_array_init(&mime_type_list);
  }
}
