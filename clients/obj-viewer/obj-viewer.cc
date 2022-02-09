#include "obj-viewer.h"

#include <string.h>

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
    layout(location = 0) in vec4 localPosition;
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
      gl_Position = zVP * position + vec4(0.0, 0.0, -1.0, 1.0);
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

const char *fragment_shader =
    GLSL(out vec4 outputColor; in vec4 frontColor;
         void main() { outputColor = vec4(frontColor.xyz, 1.0); });

ObjViewer::ObjViewer(zukou::App *app, ObjParser *obj_parser) : Background(app)
{
  parser_ = obj_parser;
}
// : app_(app), virtual_object_(virtual_object), parser_(obj_parser)

ObjViewer::~ObjViewer() {}

bool
ObjViewer::Render()
{
  for (auto obj : parser_->obj_list()) {
    // uint32_t triangle_count = 0;
    // for (auto face : obj.faces) {
    // triangle_count += face.size() - 2;
    // }
    int vertex_count = 0;
    for (auto face : obj.faces) vertex_count += face.size();

    zukou::OpenGLComponent *component = new zukou::OpenGLComponent(app_, this);
    zukou::OpenGLVertexBuffer *vertex_buffer =
        new zukou::OpenGLVertexBuffer(app_, sizeof(Vertex) * vertex_count);
    zukou::OpenGLShaderProgram *shader = new zukou::OpenGLShaderProgram(app_);

    if (obj.mtl_name == "" || parser_->mtl_table().count(obj.mtl_name) == 0) {
      std::cerr << "material not found: " << obj.mtl_name << std::endl;
      return false;
    }

    MtlObject mtl = parser_->mtl_table()[obj.mtl_name];

    Vertex *data = (Vertex *)vertex_buffer->data();

    for (auto face : obj.faces) {
      for (auto f : face) {
        data->point = parser_->vertices()[f.vertex_index];
        data->norm = parser_->norms()[f.norm_index];

        // if (obj.name == "Plane") {
        // std::cout << "point: (";
        // std::cout << data->point[0] << ", ";
        // std::cout << data->point[1] << ", ";
        // std::cout << data->point[2] << "";
        // std::cout << ") ";
        // std::cout << "norm: (";
        // std::cout << data->norm[0] << ", ";
        // std::cout << data->norm[1] << ", ";
        // std::cout << data->norm[2] << "";
        // std::cout << ")";
        // std::cout << std::endl;
        // }

        data++;
      }
    }

    // std::cout << "triangle size: " << triangle_count << std::endl;

    shader->SetVertexShader(vertex_shader, strlen(vertex_shader));
    shader->SetFragmentShader(fragment_shader, strlen(fragment_shader));
    shader->Link();
    // shader->SetUniformVariable("ambient", mtl.ambient);
    // shader->SetUniformVariable("diffuse", mtl.diffuse);
    // shader->SetUniformVariable("emissive", mtl.emissive);
    // shader->SetUniformVariable("specular", mtl.specular);
    // shader->SetUniformVariable("shininess", mtl.shininess);
    // shader->SetUniformVariable("opacity", mtl.opacity);

    component->Attach(shader);
    component->SetCount(vertex_count);
    // component->SetTopology(ZGN_OPENGL_TOPOLOGY_TRIANGLES);
    component->SetTopology(ZGN_OPENGL_TOPOLOGY_LINES);
    component->AddVertexAttribute(0, 3, ZGN_OPENGL_VERTEX_ATTRIBUTE_TYPE_FLOAT,
        false, sizeof(Vertex), offsetof(Vertex, point));
    component->AddVertexAttribute(1, 3, ZGN_OPENGL_VERTEX_ATTRIBUTE_TYPE_FLOAT,
        false, sizeof(Vertex), offsetof(Vertex, norm));

    vertex_buffer->BufferUpdated();
    component->Attach(vertex_buffer);
  }

  return true;
}
