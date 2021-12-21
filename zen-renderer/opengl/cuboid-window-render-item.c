#include "cuboid-window-render-item.h"

#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>
#include <zen-renderer/opengl-renderer.h>
#include <zen-shell/zen-shell.h>

#include "shader-compiler.h"

static void update_vertex_buffer(
    struct zen_opengl_cuboid_window_render_item* render_item);

static void
zen_opengl_cuboid_window_render_item_commit(
    struct zen_render_item* render_item_base)
{
  struct zen_opengl_cuboid_window_render_item* render_item;
  struct zen_ray* ray;
  struct zen_virtual_object* focus_virtual_object = NULL;

  render_item = wl_container_of(render_item_base, render_item, base);

  ray = render_item->cuboid_window->virtual_object->compositor->seat->ray;

  if (ray)
    focus_virtual_object =
        zen_weak_link_get_user_data(&ray->seat->ray->focus_virtual_object_link);

  if (render_item->cuboid_window->virtual_object == focus_virtual_object)
    render_item->show = true;
  else
    render_item->show = false;

  update_vertex_buffer(render_item);
}

WL_EXPORT void
zen_opengl_cuboid_window_render_item_render(
    struct zen_opengl_cuboid_window_render_item* render_item,
    struct zen_opengl_renderer_camera* camera)
{
  mat4 mvp;
  if (!render_item->show) return;

  glLineWidth(3);
  zen_virtual_object_get_model_matrix(
      render_item->cuboid_window->virtual_object, mvp);
  glm_mat4_mul(camera->view_matrix, mvp, mvp);
  glm_mat4_mul(camera->projection_matrix, mvp, mvp);

  glBindVertexArray(render_item->vertex_array_id);
  glUseProgram(render_item->program_id);
  GLint mvp_matrix_location =
      glGetUniformLocation(render_item->program_id, "mvp");
  glUniformMatrix4fv(mvp_matrix_location, 1, GL_FALSE, (float*)mvp);
  glDrawArrays(GL_LINES, 0, 48);
  glUseProgram(0);
  glBindVertexArray(0);
  glLineWidth(1);
}

WL_EXPORT struct zen_render_item*
zen_cuboid_window_render_item_create(
    struct zen_renderer* renderer_base, struct zen_cuboid_window* cuboid_window)
{
  struct zen_opengl_renderer* renderer;
  struct zen_opengl_cuboid_window_render_item* render_item;
  struct zen_opengl_shader_compiler_input shader_input;

  renderer = zen_opengl_renderer_get(renderer_base);

  render_item = zalloc(sizeof *render_item);
  if (render_item == NULL) {
    zen_log("opengl cuboid window renderer: failed to allocate memory\n");
    goto err;
  }

  shader_input.fragment_shader = zen_opengl_default_color_fragment_shader;
  shader_input.vertex_shader = zen_opengl_default_vertex_shader;
  if (zen_opengl_shader_compiler_compile(&shader_input) != 0) {
    zen_log("opengl cuboid window renderer: failed to compile shaders\n");
    goto err_shader;
  }

  render_item->base.commit = zen_opengl_cuboid_window_render_item_commit;
  render_item->renderer = renderer;
  render_item->cuboid_window = cuboid_window;
  wl_list_insert(&renderer->cuboid_window_render_item_list, &render_item->link);

  glGenVertexArrays(1, &render_item->vertex_array_id);
  glGenBuffers(1, &render_item->vertex_buffer_id);
  render_item->program_id = shader_input.program_id;
  render_item->show = false;
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
zen_cuboid_window_render_item_destroy(struct zen_render_item* render_item_base)
{
  struct zen_opengl_cuboid_window_render_item* render_item;

  render_item = wl_container_of(render_item_base, render_item, base);
  glDeleteProgram(render_item->program_id);
  glDeleteBuffers(1, &render_item->vertex_buffer_id);
  glDeleteVertexArrays(1, &render_item->vertex_array_id);
  wl_list_remove(&render_item->link);

  free(render_item);
}

static void
update_vertex_buffer(struct zen_opengl_cuboid_window_render_item* render_item)
{
  float frame_length = 0.25f;
  float r = 1 - frame_length;

  vec3 p;
  int i = 0;
  for (int j = 0; j < 3; j++) {
    int x = j;
    int y = (j + 1) % 3;
    int z = (j + 2) % 3;
    for (int k = -1; k < 2; k += 2) {
      p[y] = render_item->cuboid_window->half_size[y] * k;
      for (int l = -1; l < 2; l += 2) {
        p[z] = render_item->cuboid_window->half_size[z] * l;
        for (int m = -1; m < 2; m += 2) {
          p[x] = render_item->cuboid_window->half_size[x] * m;
          glm_vec3_copy(p, render_item->vertex_buffer[i++]);
          p[x] = render_item->cuboid_window->half_size[x] * m * r;
          glm_vec3_copy(p, render_item->vertex_buffer[i++]);
        }
      }
    }
  }

  glBindBuffer(GL_ARRAY_BUFFER, render_item->vertex_buffer_id);
  glBufferData(GL_ARRAY_BUFFER, sizeof render_item->vertex_buffer,
      render_item->vertex_buffer[0], GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

//                                         /
//                                        /
//                     .--------        ---------.
//                    /|                /       /|
//                   / |        ^y     /       / |
//                     |        |     /          |
//                              |    /
//                              |   /
//               /              |  /       /
//              /      |        | /       /      |
//             .---------       ---------.       |
//  -----------|--------------- / -------|------------>x
//             |       .-------/-       -|-------.
//             |      /       / |        |      /
//                   /       /  |              /
//                          /   |
//                         /    |
//             |          /     |        |
//             | /       /z+    |        | /
//             |/               |        |/
//             .---------       ---------.
//                              |
//                              |
//                              |
