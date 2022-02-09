#include "obj-viewer.h"

#include <linux/input-event-codes.h>
#include <string.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>

#include "types.h"

#define GLSL(str) (const char *)"#version 410\n" #str

// const char *vertex_shader =
//     GLSL(uniform mat4 model; uniform mat4 zModel; uniform mat4 zVP;
//          layout(location = 0) in vec3 localPosition;
//          layout(location = 1) in vec3 normUnnormalized; varying vec4
//          position; varying vec3 norm; void main() {
//            norm = normalize(model * vec4(normUnnormalized, 0)).xyz;
//            position = zModel * model * vec4(localPosition, 1);
//            gl_Position = zVP * position;
//          });

const char *vertex_shader = GLSL(
    struct ZLight {
      vec4 position;
      vec4 diffuse;
      vec4 ambient;
      vec4 specular;
    };
    uniform mat4 zModel; uniform mat4 zVP; uniform ZLight zLight;
    uniform mat4 model; layout(location = 0) in vec4 localPosition;
    layout(location = 1) in vec3 norm; out vec4 frontColor; void main() {
      vec4 position = zModel * localPosition;
      vec3 view = -normalize(position.xyz);
      vec3 light = normalize(
          (zLight.position * position.w - zLight.position.w * position).xyz);
      vec3 halfway = normalize(light + view);
      float diffuse = max(dot(light, norm), 0.0);
      float specular = max(dot(norm, halfway), 0.0);
      frontColor = (zLight.diffuse * 0.5 * diffuse +
                    zLight.specular * 0.5 * specular + zLight.ambient / 10);
      gl_Position = zVP * model * position;
    });

// const char *fragment_shader =
//     GLSL(uniform vec3 ambient; uniform vec3 diffuse; uniform vec3 emissive;
//          uniform vec3 specular; uniform float shininess; uniform float
//          opacity; in vec4 v_color;

//          vec3 effectiveDiffuse = diffuse * v_color.rgb;
//          float effectiveOpacity = opacity * v_color.a;

//          void main() {
//            gl_FragColor = vec4(emissive + ambient * u_ambientLight +
//                                    effectiveDiffuse * fakeLight +
//                                    specular * pow(specularLight, shininess),
//                effectiveOpacity);
//          });

const char *fragment_shader = GLSL(
    uniform vec3 ambient; uniform vec3 diffuse; uniform vec3 specular;
    out vec4 outputColor; in vec4 frontColor; void main() {
      outputColor = vec4((ambient + diffuse + specular) * frontColor.xyz, 1.0);
    });

ObjViewer::ObjViewer(zukou::App *app, ObjParser *obj_parser)
    : CuboidWindow(app, glm::vec3(1.0)), min_(FLT_MAX), max_(FLT_MIN)
{
  parser_ = obj_parser;

  for (auto obj : parser_->obj_list()) {
    std::vector<Vertex> buffer;
    for (auto face : obj.faces) {
      for (size_t i = 2; i < face.size(); i++) {
        ObjFacePoint face_point = face[0];
        buffer.push_back(Vertex(parser_->vertices()[face_point.vertex_index],
            parser_->norms()[face_point.norm_index]));

        face_point = face[i - 1];
        buffer.push_back(Vertex(parser_->vertices()[face_point.vertex_index],
            parser_->norms()[face_point.norm_index]));

        face_point = face[i];
        buffer.push_back(Vertex(parser_->vertices()[face_point.vertex_index],
            parser_->norms()[face_point.norm_index]));
      }
    }

    zukou::OpenGLComponent *component = new zukou::OpenGLComponent(app_, this);
    zukou::OpenGLVertexBuffer *vertex_buffer =
        new zukou::OpenGLVertexBuffer(app_, sizeof(Vertex) * buffer.size());
    zukou::OpenGLShaderProgram *shader = new zukou::OpenGLShaderProgram(app_);

    if (obj.mtl_name == "" || parser_->mtl_table().count(obj.mtl_name) == 0) {
      std::cerr << "material not found: " << obj.mtl_name << std::endl;
      return;
    }

    MtlObject mtl = parser_->mtl_table()[obj.mtl_name];

    shader->SetVertexShader(vertex_shader, strlen(vertex_shader));
    shader->SetFragmentShader(fragment_shader, strlen(fragment_shader));
    shader->Link();
    shader->SetUniformVariable("ambient", mtl.ambient);
    shader->SetUniformVariable("diffuse", mtl.diffuse);
    // shader->SetUniformVariable("emissive", mtl.emissive);
    shader->SetUniformVariable("specular", mtl.specular);
    // shader->SetUniformVariable("shininess", mtl.shininess);
    // shader->SetUniformVariable("opacity", mtl.opacity);
    shader->SetUniformVariable("model", glm::mat4(1));

    component->Attach(shader);
    component->SetCount(buffer.size());
    component->SetTopology(ZGN_OPENGL_TOPOLOGY_TRIANGLES);
    component->AddVertexAttribute(0, 3, ZGN_OPENGL_VERTEX_ATTRIBUTE_TYPE_FLOAT,
        false, sizeof(Vertex), offsetof(Vertex, point));
    component->AddVertexAttribute(1, 3, ZGN_OPENGL_VERTEX_ATTRIBUTE_TYPE_FLOAT,
        false, sizeof(Vertex), offsetof(Vertex, norm));

    {
      Vertex *data = (Vertex *)vertex_buffer->data();
      memcpy(data, buffer.data(), sizeof(Vertex) * buffer.size());

      vertex_buffer->BufferUpdated();
      component->Attach(vertex_buffer);
    }

    this->component_list_.push_back({
        component,
        vertex_buffer,
        shader,
    });
  }
}

ObjViewer::~ObjViewer() {}

void
ObjViewer::Configure(uint32_t serial, glm::vec3 half_size, glm::quat quaternion)
{
  zgn_cuboid_window_ack_configure(cuboid_window(), serial);
  half_size_ = half_size;
  quaternion_ = quaternion;
  // glm::mat4 model = glm::mat4(1);
  // glm::vec3 delta(0);

  // float zoom = FLT_MAX;
  // for (int i = 0; i < 3; i++) {
  //   delta[i] = (max_[i] + min_[i]) / 2;

  //   float l = max_[i] - min_[i];
  //   float r = half_size_[i] * 2 / l;
  //   if (r < zoom) zoom = r;
  // }

  // model = glm::rotate(model, -(float)M_PI / 2, glm::vec3(1.0, 0.0, 0.0));
  // model = glm::rotate(model, -(float)M_PI / 2, glm::vec3(0.0, 0.0, 1.0));
  // model = glm::scale(model, glm::vec3(zoom));
  // model = glm::translate(model, -delta);

  // for (auto component_object : component_list_) {
  //   component_object.shader->SetUniformVariable("model", model);
  //   component_object.component->Attach(component_object.shader);
  // }

  this->Commit();
}

void
ObjViewer::RayButton(uint32_t serial, uint32_t time, uint32_t button,
    enum zgn_ray_button_state state)
{
  (void)time;
  (void)button;
  if (state == ZGN_RAY_BUTTON_STATE_PRESSED) {
    zgn_cuboid_window_move(cuboid_window(), app()->seat(), serial);
  }
}
