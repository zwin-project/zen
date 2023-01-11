#include "tip-unit.h"

#include <GLES3/gl32.h>
#include <cglm/affine.h>
#include <cglm/mat4.h>
#include <cglm/vec2.h>
#include <cglm/vec3.h>

#include "zen-common.h"

/**
 * * Origin is identical with ray's tip
 * * -z direction is identical with ray's direction
 */

#define RESOLUTION_Z 50
#define RESOLUTION_THETA 25

static const int32_t vertex_count = (RESOLUTION_THETA + 1) * (RESOLUTION_Z + 1);
static const int32_t element_count =
    3 * (RESOLUTION_THETA * 2) * (RESOLUTION_Z - 1);

struct zna_ray_tip_unit_vertex {
  vec3 position;
  vec3 normal;
  vec2 uv;
};

static void
curve_ellipse(
    float t, float z0, float r0, float kz, float kr, float radius, vec2 zr)
{
  zr[0] = z0 + radius / kz * cosf(t);
  zr[1] = r0 + radius / kr * sinf(t);
}

static float
curve_modification(float z)
{
  return 10 * powf((z - 0.06) * 21.3, 30) + 0.03;
}

static void
curve_position(float t, vec2 zr)
{
  float z0 = 0.06f;
  float r0 = 0.f;
  float kz = 0.5f;
  float kr = 1.f;
  float radius = 0.02f;
  curve_ellipse(t, z0, r0, kz, kr, radius, zr);
  zr[1] *= curve_modification(zr[0]);
}

static void
curve_function(int i, vec2 zr, vec2 normal)
{
  vec2 zr_a, zr_b;
  float t = M_PI * (float)i / (float)RESOLUTION_Z;
  float t_a = M_PI * ((float)i - 0.5) / (float)RESOLUTION_Z;
  float t_b = M_PI * ((float)i + 0.5) / (float)RESOLUTION_Z;

  curve_position(t, zr);
  curve_position(t_a, zr_a);
  curve_position(t_b, zr_b);

  float dz = zr_b[0] - zr_a[0];
  float dr = zr_b[1] - zr_a[1];

  normal[0] = dr;
  normal[1] = -dz;
  glm_vec2_normalize(normal);
}

static void
construct_vertices(struct zna_ray_tip_unit_vertex *vertices)
{
  for (int i = 0; i <= RESOLUTION_Z; i++) {
    vec2 normal_zr;  // normal in zr-plane
    vec2 position_zr;
    curve_function(i, position_zr, normal_zr);

    for (int j = 0; j <= RESOLUTION_THETA; j++) {
      float theta = 2.f * M_PIf * (float)j / (float)RESOLUTION_THETA;
      float x = position_zr[1] * cosf(theta);
      float y = position_zr[1] * sinf(theta);

      float normal_x = normal_zr[1] * cosf(theta);
      float normal_y = normal_zr[1] * sinf(theta);

      struct zna_ray_tip_unit_vertex *v =
          vertices + j + i * (RESOLUTION_THETA + 1);

      v->position[0] = x;
      v->position[1] = y;
      v->position[2] = position_zr[0];

      v->normal[0] = normal_x;
      v->normal[1] = normal_y;
      v->normal[2] = normal_zr[0];

      v->uv[0] = 0;
      v->uv[1] = 0;
    }
  }
}

static void
construct_elements(short *elements)
{
  ushort num_vertices_in_a_latitudinal_line = RESOLUTION_THETA + 1;
  int max = 0;
  int count = 0;
  for (int i = 1; i < RESOLUTION_Z; i++) {
    for (int j = 0; j < RESOLUTION_THETA; j++) {
      ushort A = i * num_vertices_in_a_latitudinal_line + j;
      ushort B = A + 1;
      ushort C = A + num_vertices_in_a_latitudinal_line;
      ushort D = B - num_vertices_in_a_latitudinal_line;
      if (C > max) max = C;

      elements[count++] = A;
      elements[count++] = C;
      elements[count++] = B;

      elements[count++] = B;
      elements[count++] = D;
      elements[count++] = A;
    }
  }
}

void
zna_ray_tip_unit_commit(struct zna_ray_tip_unit *self, struct zn_ray *ray,
    struct znr_virtual_object *virtual_object)
{
  if (!self->base_unit->has_renderer_objects) return;

  mat4 local_model = GLM_MAT4_IDENTITY_INIT;

  glm_translate(local_model, ray->origin);
  glm_rotate_y(
      local_model, atan2(-ray->direction[0], -ray->direction[2]), local_model);
  glm_rotate_x(local_model, asinf(ray->direction[1]), local_model);
  glm_translate_z(local_model, -ray->length);

  znr_gl_base_technique_gl_uniform_matrix(self->base_unit->technique, 0,
      "local_model", 4, 4, 1, false, local_model[0]);

  znr_virtual_object_commit(virtual_object);
}

void
zna_ray_tip_unit_setup_renderer_objects(struct zna_ray_tip_unit *self,
    struct znr_dispatcher *dispatcher,
    struct znr_virtual_object *virtual_object)
{
  zna_base_unit_setup_renderer_objects(
      self->base_unit, dispatcher, virtual_object);
}

void
zna_ray_tip_unit_teardown_renderer_objects(struct zna_ray_tip_unit *self)
{
  zna_base_unit_teardown_renderer_objects(self->base_unit);
}

struct zna_ray_tip_unit *
zna_ray_tip_unit_create(struct zna_system *system)
{
  struct zna_ray_tip_unit *self;
  struct zna_ray_tip_unit_vertex vertices[vertex_count];
  short elements[element_count];

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  construct_vertices(vertices);
  construct_elements(elements);

  struct zgnr_mem_storage *vertex_buffer =
      zgnr_mem_storage_create(vertices, sizeof(vertices));

  struct zgnr_mem_storage *element_array_buffer =
      zgnr_mem_storage_create(elements, sizeof(elements));

  struct wl_array vertex_attributes;
  wl_array_init(&vertex_attributes);

  struct zna_base_unit_vertex_attribute *vertex_attribute;

  vertex_attribute = wl_array_add(&vertex_attributes, sizeof *vertex_attribute);
  vertex_attribute->index = 0;
  vertex_attribute->size = 3;
  vertex_attribute->type = GL_FLOAT;
  vertex_attribute->normalized = GL_FALSE;
  vertex_attribute->stride = sizeof(struct zna_ray_tip_unit_vertex);
  vertex_attribute->offset = 0;

  vertex_attribute = wl_array_add(&vertex_attributes, sizeof *vertex_attribute);
  vertex_attribute->index = 1;
  vertex_attribute->size = 3;
  vertex_attribute->type = GL_FLOAT;
  vertex_attribute->normalized = GL_FALSE;
  vertex_attribute->stride = sizeof(struct zna_ray_tip_unit_vertex);
  vertex_attribute->offset = offsetof(struct zna_ray_tip_unit_vertex, normal);

  union zgnr_gl_base_technique_draw_args draw_args;
  draw_args.elements.mode = GL_TRIANGLES;
  draw_args.elements.type = GL_UNSIGNED_SHORT;
  draw_args.elements.offset = 0;
  draw_args.elements.count = element_count;

  self->base_unit = zna_base_unit_create(system, ZNA_SHADER_RAY_VERTEX,
      ZNA_SHADER_RAY_FRAGMENT, vertex_buffer, &vertex_attributes,
      element_array_buffer, ZGNR_GL_BASE_TECHNIQUE_DRAW_METHOD_ELEMENTS,
      draw_args);

  wl_array_release(&vertex_attributes);

  zgnr_mem_storage_unref(vertex_buffer);
  zgnr_mem_storage_unref(element_array_buffer);

  return self;

err:
  return NULL;
}

void
zna_ray_tip_unit_destroy(struct zna_ray_tip_unit *self)
{
  zna_base_unit_destroy(self->base_unit);
  free(self);
}
