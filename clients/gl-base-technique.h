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
class GlBuffer;
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

  template <glm::length_t C, glm::length_t R>
  void Uniform(uint32_t location, std::string name, glm::mat<C, R, float> value)
  {
    static_assert(1 < C && C <= 4 && 1 < R && R <= 4,
        "Matrix must be between 2x2 and 4x4");

    UniformMatrix(location, std::move(name), C, R, 1, false, &value);
  }

  void DrawArrays(GLenum mode, GLint first, GLsizei count);

  void DrawElements(GLenum mode, GLsizei count, GLenum type, size_t offset,
      GlBuffer *element_array_buffer);

  void Bind(GlProgram *program);

  void Bind(GlVertexArray *vertex_array);

  void Bind(
      uint32_t binding, std::string name, GlTexture *texture, GLenum target);

 private:
  void UniformVector(uint32_t location, std::string name,
      enum zgn_gl_base_technique_uniform_variable_type type, uint32_t size,
      uint32_t count, void *value);

  void UniformMatrix(uint32_t location, std::string name, uint32_t col,
      uint32_t row, uint32_t count, bool transpose, void *value);

  Application *app_;
  zgn_gl_base_technique *proxy_ = nullptr;  // nonnull after initialization
};

std::unique_ptr<GlBaseTechnique> CreateGlBaseTechnique(
    Application *app, RenderingUnit *unit);

}  // namespace zen::client
