#include "node.h"

#include <cglm/affine.h>
#include <cglm/quat.h>
#include <zen-common.h>

#include "zgnr/intersection.h"

float
zgnr_region_node_ray_cast(struct zgnr_region_node* self, mat4 outer_transform,
    vec3 origin, vec3 direction)
{
  struct zgnr_cuboid_region* cuboid_region;
  struct zgnr_sphere_region* sphere_region;

  float min_distance = FLT_MAX;
  wl_list_for_each (cuboid_region, &self->cuboid_list, link) {
    float distance;

    mat4 transform = GLM_MAT4_IDENTITY_INIT;
    glm_translate(transform, cuboid_region->center);
    glm_quat_rotate(transform, cuboid_region->quaternion, transform);
    glm_mat4_mul(outer_transform, transform, transform);

    distance = zgnr_intersection_ray_obb(
        origin, direction, cuboid_region->half_size, transform);

    if (distance < min_distance) min_distance = distance;
  }

  wl_list_for_each (sphere_region, &self->sphere_list, link) {
    float distance;

    vec3 center;
    glm_mat4_mulv3(outer_transform, sphere_region->center, 1, center);

    distance = zgnr_intersection_ray_sphere(
        origin, direction, center, sphere_region->radius);

    if (distance < min_distance) min_distance = distance;
  }

  return min_distance;
}

void
zgnr_region_node_add_cuboid(
    struct zgnr_region_node* self, struct zgnr_cuboid_region* cuboid)
{
  wl_list_insert(&self->cuboid_list, &cuboid->link);
}

void
zgnr_region_node_add_sphere(
    struct zgnr_region_node* self, struct zgnr_sphere_region* sphere)
{
  wl_list_insert(&self->sphere_list, &sphere->link);
}

struct zgnr_region_node*
zgnr_region_node_create_copy(struct zgnr_region_node* self)
{
  struct zgnr_region_node* copy = zgnr_region_node_create();

  struct zgnr_cuboid_region* cuboid;
  wl_list_for_each (cuboid, &self->cuboid_list, link) {
    struct zgnr_cuboid_region* cuboid_copy = zgnr_cuboid_region_create(
        cuboid->half_size, cuboid->center, cuboid->quaternion);
    zgnr_region_node_add_cuboid(copy, cuboid_copy);
  }

  return copy;
}

struct zgnr_region_node*
zgnr_region_node_create(void)
{
  struct zgnr_region_node* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wl_list_init(&self->cuboid_list);
  wl_list_init(&self->sphere_list);

  return self;

err:
  return NULL;
}

void
zgnr_region_node_destroy(struct zgnr_region_node* self)
{
  struct zgnr_cuboid_region *cuboid, *tmp_cuboid;
  struct zgnr_sphere_region *sphere, *tmp_sphere;

  wl_list_for_each_safe (cuboid, tmp_cuboid, &self->cuboid_list, link) {
    zgnr_cuboid_region_destroy(cuboid);
  }

  wl_list_for_each_safe (sphere, tmp_sphere, &self->sphere_list, link) {
    zgnr_sphere_region_destroy(sphere);
  }

  free(self);
}
