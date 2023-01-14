#include "origin-unit.h"

#include <GLES3/gl32.h>
#include <cglm/affine.h>
#include <cglm/mat4.h>
#include <cglm/vec2.h>
#include <cglm/vec3.h>

#include "zen-common.h"

/**
 * * Rotate around z axis
 * * Origin is identical with ray's origin
 * * -z direction is identical with ray's direction
 * * Between north and south poll, it is divided into RESOLUTION_Z fragments.
 * * z value of north poll is CURVE_MAX_Z
 * * z value of south poll is CURVE_MIN_Z
 *
 *               ^r
 *               |
 *               |
 *          _____|___
 *         /     |   \___
 *        /      |       ---_____
 * z<----|-------|----------------\-----
 *     north     |              south
 *     pole      |              poll
 *               |
 *
 * The north and south poles and the longitudinal line at the texture juncture
 * have duplicated vertices. The number of total vertices is
 *
 *            (RESOLUTION_THETA + 1) * (RESOLUTION_Z + 1)
 *
 * ex) RESOLUTION_THETA = 8, RESOLUTION_Z = 6
 *
 * i=0 v=0  0----1----2----3----4----5----6----7----8  north pole
 * i=1      9----10---11---12---13---14---15---16---17
 * i=2      18---19---20---21---22---23---24---25---26
 * i=3      27---28---29---30---31---32---33---34---35
 * i=4      36---37---38---39---40---41---42---43---44
 * i=5      45---46---47---48---49---50---51---52---53
 * i=6 v=1  54---55---56---57---58---59---60---61---62 south pole
 *
 *          j=0  j=1  j=2  j=3  j=4  j=5  j=6  j=7  j=8
 *          u=0                                     u=1
 *
 * Vertices are stored in this order.
 * We use i,n,j like above to represent longitudinal and latitudinal lines.
 *
 */

#define RESOLUTION_Z 50
#define RESOLUTION_THETA 25

static const int32_t vertex_count = (RESOLUTION_THETA + 1) * (RESOLUTION_Z + 1);
static const int32_t element_count =
    3 * (RESOLUTION_THETA * 2) * (RESOLUTION_Z - 1);

struct zna_ray_origin_unit_vertex {
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
  return 0.7f - 40000.f * z * (z + 0.038) * (z - 0.02);
}

static void
curve_position(float t, vec2 zr)
{
  float z0 = -0.01f;
  float r0 = 0.f;
  float kz = 0.5f;
  float kr = 1.f;
  float radius = 0.01f;
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
construct_vertices(struct zna_ray_origin_unit_vertex *vertices)
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

      struct zna_ray_origin_unit_vertex *v =
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

  //         j    j+1
  //         .     .
  // i-1 . . . . . D . .
  //         .     .
  //         .     .
  // i . . . A . . B . .
  //         .     .
  //         .     .
  // i+1 . . C . . . . .
  //         .     .
  //         .     .
}

void
zna_ray_origin_unit_commit(struct zna_ray_origin_unit *self, struct zn_ray *ray,
    struct znr_virtual_object *virtual_object)
{
  if (!self->base_unit->has_renderer_objects) return;

  mat4 local_model = GLM_MAT4_IDENTITY_INIT;

  glm_translate(local_model, ray->origin);
  glm_rotate_y(
      local_model, atan2(-ray->direction[0], -ray->direction[2]), local_model);
  glm_rotate_x(local_model, asinf(ray->direction[1]), local_model);

  znr_gl_base_technique_gl_uniform_matrix(self->base_unit->technique, 0,
      "local_model", 4, 4, 1, false, local_model[0]);

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
  struct zna_ray_origin_unit_vertex vertices[vertex_count];
  short elements[element_count];

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  construct_vertices(vertices);
  construct_elements(elements);

  struct zwnr_mem_storage *vertex_buffer =
      zwnr_mem_storage_create(vertices, sizeof(vertices));

  struct zwnr_mem_storage *element_array_buffer =
      zwnr_mem_storage_create(elements, sizeof(elements));

  struct wl_array vertex_attributes;
  wl_array_init(&vertex_attributes);

  struct zna_base_unit_vertex_attribute *vertex_attribute;

  vertex_attribute = wl_array_add(&vertex_attributes, sizeof *vertex_attribute);
  vertex_attribute->index = 0;
  vertex_attribute->size = 3;
  vertex_attribute->type = GL_FLOAT;
  vertex_attribute->normalized = GL_FALSE;
  vertex_attribute->stride = sizeof(struct zna_ray_origin_unit_vertex);
  vertex_attribute->offset = 0;

  vertex_attribute = wl_array_add(&vertex_attributes, sizeof *vertex_attribute);
  vertex_attribute->index = 1;
  vertex_attribute->size = 3;
  vertex_attribute->type = GL_FLOAT;
  vertex_attribute->normalized = GL_FALSE;
  vertex_attribute->stride = sizeof(struct zna_ray_origin_unit_vertex);
  vertex_attribute->offset =
      offsetof(struct zna_ray_origin_unit_vertex, normal);

  union zwnr_gl_base_technique_draw_args draw_args;
  draw_args.elements.mode = GL_TRIANGLES;
  draw_args.elements.type = GL_UNSIGNED_SHORT;
  draw_args.elements.offset = 0;
  draw_args.elements.count = element_count;

  self->base_unit = zna_base_unit_create(system, ZNA_SHADER_RAY_VERTEX,
      ZNA_SHADER_RAY_FRAGMENT, vertex_buffer, &vertex_attributes,
      element_array_buffer, ZWNR_GL_BASE_TECHNIQUE_DRAW_METHOD_ELEMENTS,
      draw_args);

  wl_array_release(&vertex_attributes);

  zwnr_mem_storage_unref(vertex_buffer);
  zwnr_mem_storage_unref(element_array_buffer);

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
