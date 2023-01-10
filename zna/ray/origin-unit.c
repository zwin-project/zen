#include "origin-unit.h"

#include <GLES3/gl32.h>

#include "zen-common.h"

void
zna_ray_origin_unit_commit(struct zna_ray_origin_unit *self, struct zn_ray *ray,
    struct znr_virtual_object *virtual_object)
{
  if (!self->base_unit->has_renderer_objects) return;

  vec3 tip;
  zn_ray_get_tip(ray, tip);
  znr_gl_base_technique_gl_uniform_vector(self->base_unit->technique, 0, "tip",
      ZGN_GL_BASE_TECHNIQUE_UNIFORM_VARIABLE_TYPE_FLOAT, 3, 1, tip);

  znr_gl_base_technique_gl_uniform_vector(self->base_unit->technique, 0,
      "origin", ZGN_GL_BASE_TECHNIQUE_UNIFORM_VARIABLE_TYPE_FLOAT, 3, 1,
      ray->origin);

  znr_virtual_object_commit(virtual_object);
}

void
zna_ray_origin_unit_setup_renderer_objects(struct zna_ray_origin_unit *self,
    struct znr_dispatcher *dispatcher,
    struct znr_virtual_object *virtual_object)
{
  zna_base_unit_setup_renderer_objects(
      self->base_unit, dispatcher, virtual_object);
}

void
zna_ray_origin_unit_teardown_renderer_objects(struct zna_ray_origin_unit *self)
{
  zna_base_unit_teardown_renderer_objects(self->base_unit);
}

struct zna_ray_origin_unit *
zna_ray_origin_unit_create(struct zna_system *system)
{
  struct zna_ray_origin_unit *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  float vertices[2] = {0, 1};

  struct zgnr_mem_storage *vertex_buffer =
      zgnr_mem_storage_create(vertices, sizeof(vertices));

  struct wl_array vertex_attributes;
  wl_array_init(&vertex_attributes);

  struct zna_base_unit_vertex_attribute *vertex_attribute =
      wl_array_add(&vertex_attributes, sizeof *vertex_attribute);
  vertex_attribute->index = 0;
  vertex_attribute->size = 1;
  vertex_attribute->type = GL_FLOAT;
  vertex_attribute->normalized = GL_FALSE;
  vertex_attribute->stride = 0;
  vertex_attribute->offset = 0;

  union zgnr_gl_base_technique_draw_args draw_args;
  draw_args.arrays.mode = GL_LINES;
  draw_args.arrays.first = 0;
  draw_args.arrays.count = 2;

  self->base_unit = zna_base_unit_create(system, ZNA_SHADER_RAY_VERTEX,
      ZNA_SHADER_COLOR_FRAGMENT, vertex_buffer, &vertex_attributes,
      ZGNR_GL_BASE_TECHNIQUE_DRAW_METHOD_ARRAYS, draw_args);

  wl_array_release(&vertex_attributes);

  zgnr_mem_storage_unref(vertex_buffer);

  return self;

err:
  return NULL;
}

void
zna_ray_origin_unit_destroy(struct zna_ray_origin_unit *self)
{
  zna_base_unit_destroy(self->base_unit);
  free(self);
}
