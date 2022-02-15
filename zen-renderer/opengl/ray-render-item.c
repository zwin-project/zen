#include "ray-render-item.h"

#include <libzen-compositor/libzen-compositor.h>

#include "shader-compiler.h"

extern const char* white_fragmen_shader;

static void update_vertex_buffer(
    struct zen_opengl_ray_render_item* render_item);

static void
zen_opengl_ray_render_item_commit(struct zen_render_item* render_item_base)
{
  struct zen_opengl_ray_render_item* render_item;
  render_item = wl_container_of(render_item_base, render_item, base);
  update_vertex_buffer(render_item);
}

WL_EXPORT void
zen_opengl_ray_render_item_render(
    struct zen_opengl_ray_render_item* render_item,
    struct zen_opengl_renderer_camera* camera)
{
  mat4 mvp;
  glLineWidth(4);
  glm_mat4_copy(camera->view_matrix, mvp);
  glm_mat4_mul(camera->projection_matrix, mvp, mvp);

  glBindVertexArray(render_item->vertex_array_id);
  glUseProgram(render_item->program_id);
  GLint mvp_matrix_location =
      glGetUniformLocation(render_item->program_id, "mvp");
  glUniformMatrix4fv(mvp_matrix_location, 1, GL_FALSE, (float*)mvp);
  glDrawArrays(GL_LINES, 0, ARRAY_LENGTH(render_item->vertex_buffer));
  glBindVertexArray(0);
  glLineWidth(1);
}

WL_EXPORT struct zen_render_item*
zen_ray_render_item_create(
    struct zen_renderer* renderer_base, struct zen_ray* ray)
{
  struct zen_opengl_renderer* renderer;
  struct zen_opengl_ray_render_item* render_item;
  struct zen_opengl_shader_compiler_input shader_input;

  renderer = zen_opengl_renderer_get(renderer_base);

  render_item = zalloc(sizeof *render_item);
  if (render_item == NULL) {
    zen_log("opengl ray render item: failed to create a render item\n");
    goto err;
  }

  shader_input.vertex_shader = zen_opengl_default_vertex_shader;
  shader_input.fragment_shader = white_fragmen_shader;
  if (zen_opengl_shader_compiler_compile(&shader_input) != 0) {
    zen_log("opengl ray render item: failed to compile shaders\n");
    goto err_shader;
  }

  render_item->base.commit = zen_opengl_ray_render_item_commit;
  render_item->renderer = renderer;
  render_item->ray = ray;
  wl_list_insert(&renderer->ray_render_item_list, &render_item->link);

  glGenVertexArrays(1, &render_item->vertex_array_id);
  glGenBuffers(1, &render_item->vertex_buffer_id);
  render_item->program_id = shader_input.program_id;
  update_vertex_buffer(render_item);

  glBindVertexArray(render_item->vertex_array_id);
  glBindBuffer(GL_ARRAY_BUFFER, render_item->vertex_buffer_id);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glBindVertexArray(0);
  glDisableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  return &render_item->base;

err_shader:
  free(render_item);

err:
  return NULL;
}

WL_EXPORT void
zen_ray_render_item_destroy(struct zen_render_item* render_item_base)
{
  struct zen_opengl_ray_render_item* render_item;

  render_item = wl_container_of(render_item_base, render_item, base);

  glDeleteProgram(render_item->program_id);
  glDeleteBuffers(1, &render_item->vertex_buffer_id);
  glDeleteVertexArrays(1, &render_item->vertex_array_id);
  wl_list_remove(&render_item->link);

  free(render_item);
}

static void
update_vertex_buffer(struct zen_opengl_ray_render_item* render_item)
{
  const float length = render_item->ray->target_distance;
  vec3 target;

  float theta = render_item->ray->angle.polar;
  float phi = render_item->ray->angle.azimuthal;

  float sin_theta_length = sin(theta) * length;
  target[0] = sin_theta_length * cos(phi);
  target[1] = cos(theta) * length;
  target[2] = sin_theta_length * sin(phi);

  glm_vec3_copy(render_item->ray->origin, render_item->vertex_buffer[0]);
  glm_vec3_add(render_item->ray->origin, target, render_item->vertex_buffer[1]);

  glBindBuffer(GL_ARRAY_BUFFER, render_item->vertex_buffer_id);
  glBufferData(GL_ARRAY_BUFFER, sizeof render_item->vertex_buffer,
      render_item->vertex_buffer[0], GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

const char* white_fragmen_shader =
    "#version 410 core\n"
    "out vec4 outputColor;\n"
    "void main()\n"
    "{\n"
    "  outputColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
    "}\n";
