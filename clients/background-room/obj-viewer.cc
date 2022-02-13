#include "obj-viewer.h"

#include <linux/input-event-codes.h>
#include <string.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>

#include "png-loader.h"
#include "types.h"

#define GLSL(str) (const char *)"#version 410\n" #str

const char *vertex_shader = GLSL(

    uniform mat4 zModel; uniform mat4 zVP; uniform mat4 model;

    layout(location = 0) in vec4 localPosition;
    layout(location = 1) in vec3 normUnnormalized;
    layout(location = 2) in vec2 texCoordIn;

    out vec4 position; out vec3 norm; out vec2 texCoord;

    void main() {
      norm = normalize(model * vec4(normUnnormalized, 0)).xyz;
      position = zModel * (localPosition + vec4(2.0, 0.0, -6.0, 0.0));

      gl_Position = zVP * model * position;
      texCoord = texCoordIn;
    }

);

const char *fragment_shader = GLSL(

    struct ZLight {
      vec4 position;
      vec4 diffuse;
      vec4 ambient;
      vec4 specular;
    };

    uniform ZLight zLight;

    uniform sampler2D diffuseMap;

    uniform vec3 mtlAmbient; uniform vec3 mtlDiffuse; uniform vec3 mtlSpecular;
    uniform vec3 mtlEmissive;

    uniform float mtlShininess; uniform float mtlOpacity;

    in vec4 position; in vec3 norm; in vec2 texCoord;

    out vec4 outputColor;

    void main() {
      vec3 light = normalize(
          (zLight.position * position.w - zLight.position.w * position).xyz);
      float diffuse = max(dot(light, norm), 0.0);

      vec3 view = -normalize(position.xyz);
      vec3 halfway = normalize(light + view);
      float specular = pow(max(dot(norm, halfway), 0.0), mtlShininess);

      vec4 diffuseMapColor = texture(diffuseMap, texCoord);

      vec3 effectiveEmissive = mtlEmissive;
      vec3 effectiveDiffuse = diffuse * mtlDiffuse * diffuseMapColor.rgb;
      vec3 effectiveSpecular = specular * mtlSpecular;
      vec3 effectiveAmbient = mtlAmbient / 5.0;

      float effectiveOpacity = mtlOpacity * diffuseMapColor.a;

      outputColor = vec4(effectiveEmissive + effectiveDiffuse +
                             effectiveSpecular + effectiveAmbient,
          effectiveOpacity);
    }

);

ObjViewer::ObjViewer(zukou::App *app, ObjParser *obj_parser)
    : Background(app), min_(FLT_MAX), max_(FLT_MIN)
{
  parser_ = obj_parser;

  for (auto mtl : parser_->mtl_table()) {
    auto mtl_object = mtl.second;
    if (mtl_object.map_kd_path == "") {
      std::cerr << "[error] map_Kd is empty: " << mtl_object.name << std::endl;
      return;
    }

    std::string texture_path =
        parser_->texture_base_dir() + mtl_object.map_kd_path;
    PngLoader *png_loader = new PngLoader(texture_path.c_str());
    if (png_loader->Load() == false) {
      std::cerr << "[error] loading png is failed: " << mtl_object.map_kd_path
                << std::endl;
      return;
    }

    zukou::OpenGLTexture *texture = new zukou::OpenGLTexture(
        app, png_loader->height(), png_loader->width());
    if (png_loader->channel() != 4) {
      std::cerr << "[error] loading png channel is invalid(not 4): "
                << png_loader->channel() << std::endl;
      return;
    }

    zukou::ColorRGBA *pixel = (zukou::ColorRGBA *)texture->data();
    zukou::ColorRGBA *data = (zukou::ColorRGBA *)png_loader->data();
    memcpy(pixel, data,
        sizeof(zukou::ColorRGBA) * png_loader->height() * png_loader->width());

    texture_table_[mtl_object.name] = texture;
  }

  for (auto obj : parser_->obj_list()) {
    for (auto mtl_face : obj.face_table) {
      auto mtl_name = std::move(mtl_face.first);
      auto vertices = std::move(mtl_face.second);

      zukou::OpenGLComponent *component =
          new zukou::OpenGLComponent(app_, this);
      zukou::OpenGLVertexBuffer *vertex_buffer =
          new zukou::OpenGLVertexBuffer(app_, sizeof(Vertex) * vertices.size());
      zukou::OpenGLShaderProgram *shader = new zukou::OpenGLShaderProgram(app_);

      if (mtl_name == "" || parser_->mtl_table().count(mtl_name) == 0) {
        std::cerr << "material not found: " << mtl_name << std::endl;
        return;
      }

      MtlObject mtl = parser_->mtl_table()[mtl_name];

      shader->SetVertexShader(vertex_shader, strlen(vertex_shader));
      shader->SetFragmentShader(fragment_shader, strlen(fragment_shader));
      shader->Link();

      glm::mat4 model = glm::mat4(1);
      model = glm::rotate(model, (float)(M_PI / 4.0f), glm::vec3(0, 1, 0));
      shader->SetUniformVariable("model", model);

      shader->SetUniformVariable("mtlAmbient", mtl.ambient);
      shader->SetUniformVariable("mtlDiffuse", mtl.diffuse);
      shader->SetUniformVariable("mtlSpecular", mtl.specular);
      shader->SetUniformVariable("mtlEmissive", mtl.emissive);
      shader->SetUniformVariable("mtlShininess", mtl.shininess);
      shader->SetUniformVariable("mtlOpacity", mtl.opacity);

      component->Attach(shader);
      component->SetCount(vertices.size());
      component->SetTopology(ZGN_OPENGL_TOPOLOGY_TRIANGLES);
      component->AddVertexAttribute(0, 3,
          ZGN_OPENGL_VERTEX_ATTRIBUTE_TYPE_FLOAT, false, sizeof(Vertex),
          offsetof(Vertex, point));
      component->AddVertexAttribute(1, 3,
          ZGN_OPENGL_VERTEX_ATTRIBUTE_TYPE_FLOAT, false, sizeof(Vertex),
          offsetof(Vertex, norm));
      component->AddVertexAttribute(2, 2,
          ZGN_OPENGL_VERTEX_ATTRIBUTE_TYPE_FLOAT, false, sizeof(Vertex),
          offsetof(Vertex, texture));

      {
        Vertex *data = (Vertex *)vertex_buffer->data();
        memcpy(data, vertices.data(), sizeof(Vertex) * vertices.size());
        vertex_buffer->BufferUpdated();
        component->Attach(vertex_buffer);
      }

      {
        zukou::OpenGLTexture *texture = texture_table_[mtl.name];
        texture->BufferUpdated();
        component->Attach(texture);
      }

      this->component_list_.push_back({
          component,
          vertex_buffer,
          shader,
      });
    }
  }

  this->Commit();
}

ObjViewer::~ObjViewer() {}
