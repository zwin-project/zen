#include "nameplate.h"

#include <GLES3/gl32.h>
#include <cglm/affine.h>
#include <cglm/types.h>
#include <zen-common.h>

#include "zen/virtual-object.h"
#include "zns/appearance/bounded.h"
#include "zns/bounded-nameplate.h"

struct zna_bounded_nameplate_vertex {
  vec3 position;
  vec2 uv;
};

#define NAMEPLATE_PIXEL_PER_METER 2800.f

static void
zna_bounded_nameplate_unit_update_texture(
    struct zna_bounded_nameplate_unit *self, struct zns_bounded *bounded)
{
  int width = NAMEPLATE_PIXEL_PER_METER * bounded->nameplate->geometry.width;
  int height = NAMEPLATE_PIXEL_PER_METER * bounded->nameplate->geometry.height;

  cairo_surface_t *surface =
      cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
    zn_error("Failed to create cairo_surface");
    goto out;
    return;
  }

  cairo_t *cr = cairo_create(surface);
  if (cairo_status(cr) != CAIRO_STATUS_SUCCESS) {
    zn_error("Failed to create cairo_surface");
    goto out_cairo;
  }

  vec3 navy = ZN_NAVY_VEC3_INIT;
  cairo_set_source_rgba(cr, navy[0], navy[1], navy[2], 1.);
  zn_cairo_draw_rounded_rectangle(cr, 0, 0, width, height, 25.);
  cairo_fill(cr);
  cairo_set_font_face(cr, zn_font_face_get_cairo_font_face(ZN_FONT_REGULAR));
  cairo_set_source_rgba(cr, 1., 1., 1., 1.);
  cairo_set_font_size(cr, height / 2);
  zn_cairo_draw_text(cr, "Bounded App", 40, height / 2, ZN_CAIRO_ANCHOR_LEFT,
      ZN_CAIRO_ANCHOR_CENTER);

  zna_base_unit_read_cairo_surface(self->base_unit, surface);
  znr_gl_sampler_parameter_i(
      self->base_unit->sampler0, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  znr_gl_sampler_parameter_i(
      self->base_unit->sampler0, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

out_cairo:
  cairo_destroy(cr);

out:
  cairo_surface_destroy(surface);
}

void
zna_bounded_nameplate_unit_commit(struct zna_bounded_nameplate_unit *self,
    struct zns_bounded *bounded, struct znr_virtual_object *znr_virtual_object,
    uint32_t damage)
{
  if (!self->base_unit->has_renderer_objects) return;

  if (damage & ZNA_BOUNDED_DAMAGE_GEOMETRY) {
    mat4 local_model;
    zns_bounded_nameplate_get_transform(bounded->nameplate, local_model);

    glm_scale(local_model, (vec3){bounded->nameplate->geometry.width,
                               bounded->nameplate->geometry.height, 1});

    znr_gl_base_technique_gl_uniform_matrix(self->base_unit->technique, 0,
        "local_model", 4, 4, 1, false, local_model[0]);
  }

  if (damage & ZNA_BOUNDED_DAMAGE_NAMEPLATE_TEXTURE) {
    zna_bounded_nameplate_unit_update_texture(self, bounded);
  }

  if (damage) {
    znr_virtual_object_commit(znr_virtual_object);
  }
}

void
zna_bounded_nameplate_unit_setup_renderer_objects(
    struct zna_bounded_nameplate_unit *self, struct znr_dispatcher *dispatcher,
    struct znr_virtual_object *virtual_object)
{
  zna_base_unit_setup_renderer_objects(
      self->base_unit, dispatcher, virtual_object);
}

void
zna_bounded_nameplate_unit_teardown_renderer_objects(
    struct zna_bounded_nameplate_unit *self)
{
  zna_base_unit_teardown_renderer_objects(self->base_unit);
}

struct zna_bounded_nameplate_unit *
zna_bounded_nameplate_unit_create(struct zna_system *system)
{
  struct zna_bounded_nameplate_unit *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  struct zna_bounded_nameplate_vertex vertices[4] = {
      {{-0.5f, 0.f, 0.f}, {0.f, 0.f}},
      {{+0.5f, 0.f, 0.f}, {1.f, 0.f}},
      {{+0.5f, -1.f, 0.f}, {1.f, 1.f}},
      {{-0.5f, -1.f, 0.f}, {0.f, 1.f}},
  };

  struct zgnr_mem_storage *vertex_buffer =
      zgnr_mem_storage_create(vertices, sizeof(vertices));

  struct wl_array vertex_attributes;
  wl_array_init(&vertex_attributes);

  struct zna_base_unit_vertex_attribute *vertex_attribute;

  vertex_attribute = wl_array_add(&vertex_attributes, sizeof *vertex_attribute);
  vertex_attribute->index = 0;
  vertex_attribute->size = 3;
  vertex_attribute->type = GL_FLOAT;
  vertex_attribute->normalized = GL_FALSE;
  vertex_attribute->stride = sizeof(struct zna_bounded_nameplate_vertex);
  vertex_attribute->offset = 0;

  vertex_attribute = wl_array_add(&vertex_attributes, sizeof *vertex_attribute);
  vertex_attribute->index = 1;
  vertex_attribute->size = 2;
  vertex_attribute->type = GL_FLOAT;
  vertex_attribute->normalized = GL_FALSE;
  vertex_attribute->stride = sizeof(struct zna_bounded_nameplate_vertex);
  vertex_attribute->offset = offsetof(struct zna_bounded_nameplate_vertex, uv);

  union zgnr_gl_base_technique_draw_args draw_args;
  draw_args.arrays.mode = GL_TRIANGLE_FAN;
  draw_args.arrays.first = 0;
  draw_args.arrays.count = 4;

  self->base_unit = zna_base_unit_create(system,
      ZNA_SHADER_BOUNDED_NAMEPLATE_VERTEX,
      ZNA_SHADER_BOUNDED_NAMEPLATE_FRAGMENT, vertex_buffer, &vertex_attributes,
      NULL, ZGNR_GL_BASE_TECHNIQUE_DRAW_METHOD_ARRAYS, draw_args);

  wl_array_release(&vertex_attributes);

  zgnr_mem_storage_unref(vertex_buffer);

  return self;

err:
  return NULL;
}

void
zna_bounded_nameplate_unit_destroy(struct zna_bounded_nameplate_unit *self)
{
  zna_base_unit_destroy(self->base_unit);
  free(self);
}
