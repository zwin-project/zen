#include "node.h"

#include <cglm/affine.h>
#include <cglm/quat.h>
#include <zen-common.h>

#include "zwnr/intersection.h"

float
zwnr_region_node_ray_cast(struct zwnr_region_node *self, mat4 outer_transform,
    vec3 origin, vec3 direction)
{
  struct zwnr_cuboid_region *cuboid_region;
  float min_distance = FLT_MAX;
  wl_list_for_each (cuboid_region, &self->cuboid_list, link) {
    float distance;

    mat4 transform = GLM_MAT4_IDENTITY_INIT;
    glm_translate(transform, cuboid_region->center);
    glm_quat_rotate(transform, cuboid_region->quaternion, transform);
    glm_mat4_mul(outer_transform, transform, transform);

    distance = zwnr_intersection_ray_obb(
        origin, direction, cuboid_region->half_size, transform);

    if (distance < min_distance) min_distance = distance;
  }

  return min_distance;
}

void
zwnr_region_node_add_cuboid(
    struct zwnr_region_node *self, struct zwnr_cuboid_region *cuboid)
{
  wl_list_insert(&self->cuboid_list, &cuboid->link);
}

struct zwnr_region_node *
zwnr_region_node_create_copy(struct zwnr_region_node *self)
{
  struct zwnr_region_node *copy = zwnr_region_node_create();

  struct zwnr_cuboid_region *cuboid;
  wl_list_for_each (cuboid, &self->cuboid_list, link) {
    struct zwnr_cuboid_region *cuboid_copy = zwnr_cuboid_region_create(
        cuboid->half_size, cuboid->center, cuboid->quaternion);
    zwnr_region_node_add_cuboid(copy, cuboid_copy);
  }

  return copy;
}

struct zwnr_region_node *
zwnr_region_node_create(void)
{
  struct zwnr_region_node *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wl_list_init(&self->cuboid_list);

  return self;

err:
  return NULL;
}

void
zwnr_region_node_destroy(struct zwnr_region_node *self)
{
  struct zwnr_cuboid_region *cuboid, *tmp;

  wl_list_for_each_safe (cuboid, tmp, &self->cuboid_list, link) {
    zwnr_cuboid_region_destroy(cuboid);
  }

  free(self);
}
