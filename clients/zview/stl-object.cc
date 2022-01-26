#include <iostream>

#include "internal.h"

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
    "void main()\n"
    "{\n"
    "  outputColor = vec4(frontColor.xyz, 1.0);\n"
    "}\n");

#pragma pack(1)
struct StlRawTriangle {
  float n[3];
  float points[3][3];
  uint16_t unused;
};
#pragma pack()

StlObject::StlObject() : Drawable(), max_(FLT_MIN), min_(FLT_MAX) {}

void
StlObject::Draw(zukou::CuboidWindow *cuboid_window)
{
  zukou::App *app = cuboid_window->app();
  float zoom = FLT_MAX;
  glm::vec3 delta(0);
  StlVertex *data;

  if (!vertex_buffer_) {
    vertex_buffer_.reset(new zukou::OpenGLVertexBuffer(
        app, sizeof(StlVertex) * vertices_.size()));
  }

  if (!shader_) {
    shader_.reset(new zukou::OpenGLShaderProgram(app));
    shader_->SetVertexShader(vertex_shader.c_str(), vertex_shader.size());
    shader_->SetFragmentShader(fragment_shader.c_str(), fragment_shader.size());
    shader_->Link();
  }

  if (!component_) {
    component_.reset(new zukou::OpenGLComponent(app, cuboid_window));
    component_->Attach(shader_.get());
    component_->SetCount(vertices_.size());
    component_->SetTopology(ZGN_OPENGL_TOPOLOGY_TRIANGLES);
    component_->AddVertexAttribute(0, 3, ZGN_OPENGL_VERTEX_ATTRIBUTE_TYPE_FLOAT,
        false, sizeof(StlVertex), offsetof(StlVertex, point));
    component_->AddVertexAttribute(1, 3, ZGN_OPENGL_VERTEX_ATTRIBUTE_TYPE_FLOAT,
        false, sizeof(StlVertex), offsetof(StlVertex, norm));
  }

  for (int i = 0; i < 3; i++) {
    delta[i] = (max_[i] + min_[i]) / 2;

    float l = max_[i] - min_[i];
    float r = cuboid_window->half_size()[i] * 2 / l;
    if (r < zoom) zoom = r;
  }

  data = (StlVertex *)vertex_buffer_->data();

  for (StlVertex v : vertices_) {
    data->point = (v.point - delta) * zoom;
    data->norm = v.norm;
    data++;
  }

  vertex_buffer_->BufferUpdated();
  component_->Attach(vertex_buffer_.get());
  cuboid_window->Commit();
}

StlObject::~StlObject() {}

bool
StlObject::Fill(std::unique_ptr<Reader> reader)
{
  struct StlRawTriangle buffer;
  std::vector<StlVertex> tmp;
  uint32_t triangle_count;
  char unused[80];

  if (!reader->Read(unused, 80)) return false;

  if (std::string(unused, 5) == "solid") {
    std::cerr << "the content seems \"ASCII\" STL which is not "
                 "supported yet"
              << std::endl;
    return false;
  }

  if (!reader->Read(
          reinterpret_cast<char *>(&triangle_count), sizeof(triangle_count)))
    return false;

  if (triangle_count <= 0) {
    std::cerr << "broken file" << std::endl;
    return false;
  }

  tmp.reserve(triangle_count * 3);

  for (uint32_t i = 0; i < triangle_count; i++) {
    if (!reader->Read(
            reinterpret_cast<char *>(&buffer), sizeof(StlRawTriangle))) {
      std::cerr << "broken file" << std::endl;
      return false;
    }

    // we assume stl's z axis as y axis in zigen
    for (int i = 0; i < 3; i++) {
      float *p = buffer.points[i];
      if (p[0] < min_.x) min_.x = p[0];
      if (p[2] < min_.y) min_.y = p[2];
      if (-p[1] < min_.z) min_.z = -p[1];
      if (p[0] > max_.x) max_.x = p[0];
      if (p[2] > max_.y) max_.y = p[2];
      if (-p[1] > max_.z) max_.z = -p[1];
      tmp.push_back(StlVertex({
          glm::vec3(buffer.n[0], buffer.n[2], -buffer.n[1]),
          glm::vec3(
              buffer.points[i][0], buffer.points[i][2], -buffer.points[i][1]),
      }));
    }
  }

  this->vertices_.swap(tmp);

  return true;
}
