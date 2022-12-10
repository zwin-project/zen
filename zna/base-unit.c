#include "base-unit.h"

#include <GL/glew.h>
#include <stdio.h>
#include <wlr/render/egl.h>
#include <wlr/render/glew.h>
#include <zen-common.h>
#include <zgnr/gl-sampler.h>

#include "zen/server.h"

void
zna_base_unit_read_wlr_texture(
    struct zna_base_unit *self, struct wlr_texture *texture)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zgnr_mem_storage *storage;

  if (!self->has_renderer_objects) return;

  if (!zn_assert(wlr_texture_is_glew(texture), "glew renderer is required")) {
    return;
  }

  storage =
      zgnr_mem_storage_create(NULL, 32 * texture->width * texture->height);

  struct wlr_glew_texture_attribs texture_attrib;
  wlr_glew_texture_get_attribs(texture, &texture_attrib);

  struct wlr_egl *egl = wlr_glew_renderer_get_egl(server->renderer);
  wlr_egl_make_current(egl);

  GLuint framebuffer;
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  GLuint depth_texture;
  {
    glGenTextures(1, &depth_texture);
    glBindTexture(GL_TEXTURE_2D, depth_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, texture->width,
        texture->height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

    glBindTexture(GL_TEXTURE_2D, 0);
  }

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
      texture_attrib.tex, 0);
  glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);

  glReadPixels(0, 0, texture->width, texture->height, GL_RGBA, GL_UNSIGNED_BYTE,
      storage->data);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteTextures(1, &depth_texture);
  glDeleteFramebuffers(1, &framebuffer);

  wlr_egl_unset_current(egl);

  if (self->has_texture_data == false) {
    self->has_texture_data = true;
    znr_gl_base_technique_bind_texture(
        self->technique, 0, "", self->texture0, GL_TEXTURE_2D, self->sampler0);
  }

  znr_gl_texture_image_2d(self->texture0, GL_TEXTURE_2D, 0, GL_RGBA,
      texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, storage);

  zgnr_mem_storage_unref(storage);
}

void
zna_base_unit_setup_renderer_objects(struct zna_base_unit *self,
    struct znr_session *session, struct znr_virtual_object *virtual_object)
{
  struct znr_gl_shader *vertex_shader, *fragment_shader;

  if (self->has_renderer_objects) return;

  self->rendering_unit = znr_rendering_unit_create(session, virtual_object);
  self->technique = znr_gl_base_technique_create(session, self->rendering_unit);
  self->vertex_buffer = znr_gl_buffer_create(session, self->system->display);
  self->vertex_array = znr_gl_vertex_array_create(session);
  self->program = znr_gl_program_create(session);
  self->texture0 = znr_gl_texture_create(session, self->system->display);
  self->sampler0 = znr_gl_sampler_create(session);

  // program
  vertex_shader = zna_shader_inventory_get(
      self->system->shader_inventory, self->vertex_shader);
  fragment_shader = zna_shader_inventory_get(
      self->system->shader_inventory, self->fragment_shader);

  znr_gl_program_attach_shader(self->program, vertex_shader);
  znr_gl_program_attach_shader(self->program, fragment_shader);
  znr_gl_program_link(self->program);

  // vertex buffer
  znr_gl_buffer_data(self->vertex_buffer, GL_ARRAY_BUFFER,
      self->vertex_buffer_storage, GL_STATIC_DRAW);

  // vertex array
  struct zna_base_unit_vertex_attribute *vertex_attribute;
  wl_array_for_each (vertex_attribute, &self->vertex_attributes) {
    znr_gl_vertex_array_enable_vertex_attrib_array(
        self->vertex_array, vertex_attribute->index);
    znr_gl_vertex_array_vertex_attrib_pointer(self->vertex_array,
        vertex_attribute->index, vertex_attribute->size, vertex_attribute->type,
        vertex_attribute->normalized, vertex_attribute->stride,
        vertex_attribute->offset, self->vertex_buffer);
  }

  // base technique
  znr_gl_base_technique_bind_vertex_array(self->technique, self->vertex_array);
  znr_gl_base_technique_bind_program(self->technique, self->program);

  switch (self->draw_method) {
    case ZGNR_GL_BASE_TECHNIQUE_DRAW_METHOD_ARRAYS:
      znr_gl_base_technique_draw_arrays(self->technique,
          self->draw_args.arrays.mode, self->draw_args.arrays.first,
          self->draw_args.arrays.count);
      break;

    case ZGNR_GL_BASE_TECHNIQUE_DRAW_METHOD_NONE:  // fall through
    default:
      break;
  }

  self->has_renderer_objects = true;
}

void
zna_base_unit_teardown_renderer_objects(struct zna_base_unit *self)
{
  if (!self->has_renderer_objects) return;

  znr_rendering_unit_destroy(self->rendering_unit);
  self->rendering_unit = NULL;
  znr_gl_base_technique_destroy(self->technique);
  self->technique = NULL;
  znr_gl_buffer_destroy(self->vertex_buffer);
  self->vertex_buffer = NULL;
  znr_gl_vertex_array_destroy(self->vertex_array);
  self->vertex_array = NULL;
  znr_gl_program_destroy(self->program);
  self->program = NULL;
  znr_gl_texture_destroy(self->texture0);
  self->texture0 = NULL;
  znr_gl_sampler_destroy(self->sampler0);
  self->sampler0 = NULL;

  self->has_renderer_objects = false;
  self->has_texture_data = false;
}

struct zna_base_unit *
zna_base_unit_create(struct zna_system *system,
    enum zna_shader_name vertex_shader, enum zna_shader_name fragment_shader,
    struct zgnr_mem_storage *vertex_buffer, struct wl_array *vertex_attributes,
    enum zgnr_gl_base_technique_draw_method draw_method,
    union zgnr_gl_base_technique_draw_args draw_args)
{
  struct zna_base_unit *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->system = system;
  self->has_renderer_objects = false;
  self->has_texture_data = false;
  self->vertex_shader = vertex_shader;
  self->fragment_shader = fragment_shader;
  self->vertex_buffer_storage = vertex_buffer;
  zgnr_mem_storage_ref(vertex_buffer);
  wl_array_init(&self->vertex_attributes);
  wl_array_copy(&self->vertex_attributes, vertex_attributes);
  self->draw_method = draw_method;
  self->draw_args = draw_args;

  return self;

err:
  return NULL;
}

void
zna_base_unit_destroy(struct zna_base_unit *self)
{
  if (self->rendering_unit) znr_rendering_unit_destroy(self->rendering_unit);
  if (self->technique) znr_gl_base_technique_destroy(self->technique);
  if (self->vertex_buffer) znr_gl_buffer_destroy(self->vertex_buffer);
  if (self->vertex_array) znr_gl_vertex_array_destroy(self->vertex_array);
  if (self->program) znr_gl_program_destroy(self->program);
  if (self->texture0) znr_gl_texture_destroy(self->texture0);
  if (self->sampler0) znr_gl_sampler_destroy(self->sampler0);

  zgnr_mem_storage_unref(self->vertex_buffer_storage);
  wl_array_release(&self->vertex_attributes);

  free(self);
}
