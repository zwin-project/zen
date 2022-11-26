#pragma once

#include <GLES3/gl32.h>
#include <zen-common.h>
#include <zigen-gles-v32-client-protocol.h>

#include <glm/detail/type_vec1.hpp>
#include <glm/detail/type_vec2.hpp>
#include <glm/detail/type_vec3.hpp>
#include <glm/detail/type_vec4.hpp>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace zen::client {

class Application;
class RenderingUnit;
class GlProgram;
class GlVertexArray;
class GlTexture;

class GlBaseTechnique
{
 public:
  DISABLE_MOVE_AND_COPY(GlBaseTechnique);
  GlBaseTechnique(Application *app);
  ~GlBaseTechnique();

  bool Init(RenderingUnit *unit);

  template <glm::length_t length, typename T>
  void Uniform(uint32_t location, std::string name, glm::vec<length, T> value)
  {
    static_assert(std::is_same<T, float>::value ||
                      std::is_same<T, int32_t>::value ||
                      std::is_same<T, uint32_t>::value,
        "Vector type must be one of [float, int32_t, uint32_t]");
    static_assert(
        0 < length && length <= 4, "Vector length must be between 1 to 4");

    zgn_gl_base_technique_uniform_variable_type type;
    if (std::is_same<T, float>::value) {
      type = ZGN_GL_BASE_TECHNIQUE_UNIFORM_VARIABLE_TYPE_FLOAT;
    } else if (std::is_same<T, int32_t>::value) {
      type = ZGN_GL_BASE_TECHNIQUE_UNIFORM_VARIABLE_TYPE_INT;
    } else if (std::is_same<T, uint32_t>::value) {
      type = ZGN_GL_BASE_TECHNIQUE_UNIFORM_VARIABLE_TYPE_UINT;
    } else {
      assert(false && "unknown vector type");
    }

    UniformVector(location, std::move(name), type, length, 1, &value);
  }

  void DrawArrays(GLenum mode, GLint first, GLsizei count);

  void Bind(GlProgram *program);

  void Bind(GlVertexArray *vertex_array);

  void Bind(
      uint32_t binding, std::string name, GlTexture *texture, GLenum target);

 private:
  void UniformVector(uint32_t location, std::string name,
      enum zgn_gl_base_technique_uniform_variable_type type, uint32_t size,
      uint32_t count, void *value);

  Application *app_;
  zgn_gl_base_technique *proxy_ = nullptr;  // nonnull after initialization
};

std::unique_ptr<GlBaseTechnique> CreateGlBaseTechnique(
    Application *app, RenderingUnit *unit);

}  // namespace zen::client
