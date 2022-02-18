#include "ray-render-item.h"

#include <libzen-compositor/libzen-compositor.h>

#include "ray-render-item-icon.h"
#include "shader-compiler.h"

extern const char* white_fragmen_shader;

static void update_vertex_buffer(
    struct zen_opengl_ray_render_item* render_item);

static void draw_dnd_icon_texture(
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
  struct zen_ray* ray = render_item->ray;

  glm_mat4_copy(camera->view_matrix, mvp);
  glm_mat4_mul(camera->projection_matrix, mvp, mvp);

  glBindVertexArray(render_item->vertex_array_id);
  glUseProgram(render_item->program_id);
  GLint mvp_matrix_location =
      glGetUniformLocation(render_item->program_id, "mvp");
  glUniformMatrix4fv(mvp_matrix_location, 1, GL_FALSE, (float*)mvp);
  glDrawArrays(GL_LINES, 0, ARRAY_LENGTH(render_item->vertex_buffer));

  if (ray->is_dragging) {
    glBindVertexArray(render_item->dnd_vertex_array_id);
    glUseProgram(render_item->dnd_program_id);
    mvp_matrix_location =
        glGetUniformLocation(render_item->dnd_program_id, "mvp");
    glUniformMatrix4fv(mvp_matrix_location, 1, GL_FALSE, (float*)mvp);
    glBindTexture(GL_TEXTURE_2D, render_item->dnd_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, ARRAY_LENGTH(render_item->dnd_vertex_buffer));
  }

  glBindVertexArray(0);
  glLineWidth(1);
}

WL_EXPORT struct zen_render_item*
zen_ray_render_item_create(
    struct zen_renderer* renderer_base, struct zen_ray* ray)
{
  struct zen_opengl_renderer* renderer;
  struct zen_opengl_ray_render_item* render_item;
  struct zen_opengl_shader_compiler_input shader_input, dnd_shader_input;

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

  dnd_shader_input.vertex_shader = zen_opengl_default_vertex_shader;
  dnd_shader_input.fragment_shader = zen_opengl_default_texture_fragment_shader;
  if (zen_opengl_shader_compiler_compile(&dnd_shader_input) != 0) {
    zen_log("opengl ray render item: failed to compile shaders\n");
    goto err_dnd_shader;
  }

  render_item->base.commit = zen_opengl_ray_render_item_commit;
  render_item->renderer = renderer;
  render_item->ray = ray;
  wl_list_insert(&renderer->ray_render_item_list, &render_item->link);

  glGenVertexArrays(1, &render_item->vertex_array_id);
  glGenBuffers(1, &render_item->vertex_buffer_id);
  glGenVertexArrays(1, &render_item->dnd_vertex_array_id);
  glGenBuffers(1, &render_item->dnd_vertex_buffer_id);
  glGenTextures(1, &render_item->dnd_texture_id);
  render_item->program_id = shader_input.program_id;
  render_item->dnd_program_id = dnd_shader_input.program_id;
  update_vertex_buffer(render_item);

  glBindVertexArray(render_item->vertex_array_id);
  glBindBuffer(GL_ARRAY_BUFFER, render_item->vertex_buffer_id);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glBindVertexArray(0);
  glDisableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindVertexArray(render_item->dnd_vertex_array_id);
  glBindBuffer(GL_ARRAY_BUFFER, render_item->dnd_vertex_buffer_id);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, 0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(
      1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));
  glBindVertexArray(0);
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  draw_dnd_icon_texture(render_item);

  return &render_item->base;

err_dnd_shader:
  glDeleteProgram(shader_input.program_id);

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

  glDeleteProgram(render_item->dnd_program_id);
  glDeleteTextures(1, &render_item->dnd_texture_id);
  glDeleteBuffers(1, &render_item->dnd_vertex_buffer_id);
  glDeleteVertexArrays(1, &render_item->dnd_vertex_array_id);

  wl_list_remove(&render_item->link);

  free(render_item);
}

static void
update_vertex_buffer(struct zen_opengl_ray_render_item* render_item)
{
  const float length = render_item->ray->target_distance;
  vec3 direction, target, horizontal, vertical, tmp;
  float theta, phi, sin_theta;

  theta = render_item->ray->angle.polar;
  phi = render_item->ray->angle.azimuthal;

  sin_theta = sin(theta);
  direction[0] = sin_theta * cos(phi);
  direction[1] = cos(theta);
  direction[2] = sin_theta * sin(phi);

  glm_vec3_copy(render_item->ray->origin, render_item->vertex_buffer[0]);
  glm_vec3_scale(direction, length - 0.05, target);
  glm_vec3_add(render_item->ray->origin, target, target);
  glm_vec3_copy(target, render_item->vertex_buffer[1]);

  glBindBuffer(GL_ARRAY_BUFFER, render_item->vertex_buffer_id);
  glBufferData(GL_ARRAY_BUFFER, sizeof render_item->vertex_buffer,
      render_item->vertex_buffer[0], GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glm_vec3_zero(horizontal);
  horizontal[1] = 0.025;
  glm_vec3_crossn(direction, horizontal, vertical);
  glm_vec3_scale(vertical, 0.025, vertical);

  glm_vec3_add(target, horizontal, tmp);
  glm_vec3_sub(tmp, vertical, render_item->dnd_vertex_buffer[0].p);
  glm_vec3_add(tmp, vertical, render_item->dnd_vertex_buffer[1].p);
  glm_vec3_add(tmp, vertical, render_item->dnd_vertex_buffer[5].p);
  glm_vec3_sub(target, horizontal, tmp);
  glm_vec3_sub(tmp, vertical, render_item->dnd_vertex_buffer[2].p);
  glm_vec3_sub(tmp, vertical, render_item->dnd_vertex_buffer[4].p);
  glm_vec3_add(tmp, vertical, render_item->dnd_vertex_buffer[3].p);

  glBindBuffer(GL_ARRAY_BUFFER, render_item->dnd_vertex_buffer_id);
  glBufferData(GL_ARRAY_BUFFER, sizeof render_item->dnd_vertex_buffer,
      render_item->dnd_vertex_buffer[0].p, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

struct bgra {
  uint8_t b, g, r, a;
};

static void
draw_dnd_icon_texture(struct zen_opengl_ray_render_item* render_item)
{
  struct bgra* colors =
      zalloc(sizeof(struct bgra) * icon_bmp_width * icon_bmp_height);

  render_item->dnd_vertex_buffer[0].uv[0] = 0;
  render_item->dnd_vertex_buffer[0].uv[1] = 1;
  render_item->dnd_vertex_buffer[1].uv[0] = 1;
  render_item->dnd_vertex_buffer[1].uv[1] = 1;
  render_item->dnd_vertex_buffer[2].uv[0] = 0;
  render_item->dnd_vertex_buffer[2].uv[1] = 0;
  render_item->dnd_vertex_buffer[3].uv[0] = 1;
  render_item->dnd_vertex_buffer[3].uv[1] = 0;
  render_item->dnd_vertex_buffer[4].uv[0] = 0;
  render_item->dnd_vertex_buffer[4].uv[1] = 0;
  render_item->dnd_vertex_buffer[5].uv[0] = 1;
  render_item->dnd_vertex_buffer[5].uv[1] = 1;

  int i = 0;
  int j = icon_bmp_width * icon_bmp_height - 1;
  for (uint32_t x = 0; x < icon_bmp_width; x++) {
    for (uint32_t y = 0; y < icon_bmp_height; y++) {
      colors[j].a = icon_bmp[i];
      colors[j].r = 0x0;
      colors[j].g = 0xff;
      colors[j].b = 0xff;
      i++;
      j--;
    }
  }

  glBindTexture(GL_TEXTURE_2D, render_item->dnd_texture_id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, icon_bmp_width, icon_bmp_height, 0,
      GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, colors);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
  free(colors);
}

const char* white_fragmen_shader =
    "#version 410 core\n"
    "out vec4 outputColor;\n"
    "void main()\n"
    "{\n"
    "  outputColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
    "}\n";
