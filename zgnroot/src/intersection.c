#include "zgnr/intersection.h"

#include <cglm/vec3.h>
#include <float.h>
#include <math.h>

static int
ray_obb_intersection_axis_test(vec3 axis, vec3 translation, vec3 ray_direction,
    float axis_min, float axis_max, float *t_min, float *t_max)
{
  float e = glm_vec3_dot(axis, translation);
  float f = glm_vec3_dot(ray_direction, axis);

  if (fabs(f) > 0.001f) {
    float t1 = (e + axis_min) / f;
    float t2 = (e + axis_max) / f;
    if (t1 > t2) glm_swapf(&t1, &t2);
    if (t2 < *t_max) *t_max = t2;
    if (*t_min < t1) *t_min = t1;
    if (*t_max < *t_min) return -1;
  } else {
    if (-e + axis_min > 0.0f || -e + axis_max < 0.0f) return -1;
  }
  return 0;
}

float
zgnr_intersection_ray_obb(
    vec3 origin, vec3 direction, vec3 aabb_half_size, mat4 transform)
{
  float t_min = 0.0f;
  float t_max = FLT_MAX;

  vec3 obb_position_worldspace, translation;
  glm_vec4_copy3(transform[3], obb_position_worldspace);
  glm_vec3_sub(obb_position_worldspace, origin, translation);

  for (int i = 0; i < 3; i++) {  // test x, y and z axis
    vec3 axis;
    glm_vec4_copy3(transform[i], axis);
    if (ray_obb_intersection_axis_test(axis, translation, direction,
            -aabb_half_size[i], aabb_half_size[i], &t_min, &t_max) == -1)
      return FLT_MAX;
  }

  return t_min;
}
