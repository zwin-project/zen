#include "zwnr/intersection.h"

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
zwnr_intersection_ray_obb(
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

/* Customize Möller–Trumbore ray-triangle intersection algorithm */
float
zwnr_intersection_ray_parallelogram(vec3 origin, vec3 direction, vec3 v0,
    vec3 v1, vec3 v2, float *u, float *v, bool test_cull)
{
  static const float epsilon = 0.000001;
  vec3 edge1, edge2, tvec, pvec, qvec;
  float det, inv_det, t;

  glm_vec3_sub(v1, v0, edge1);
  glm_vec3_sub(v2, v0, edge2);

  glm_vec3_cross(direction, edge2, pvec);
  det = glm_vec3_dot(edge1, pvec);

  if (test_cull) {
    if (det < epsilon) return FLT_MAX;

    glm_vec3_sub(origin, v0, tvec);

    *u = glm_vec3_dot(tvec, qvec);
    if (*u < 0.f || *u > det) return FLT_MAX;

    glm_vec3_cross(tvec, edge1, qvec);

    *v = glm_vec3_dot(direction, qvec);
    if (*v < 0.f || *v > det) return FLT_MAX;

    t = glm_vec3_dot(edge2, qvec);
    inv_det = 1.f / det;
    t *= inv_det;
    *u *= inv_det;
    *v *= inv_det;
  } else {
    if (det > -epsilon && det < epsilon) return FLT_MAX;

    inv_det = 1.f / det;

    glm_vec3_sub(origin, v0, tvec);

    *u = glm_vec3_dot(tvec, pvec) * inv_det;
    if (*u < 0.f || *u > 1.f) return FLT_MAX;

    glm_vec3_cross(tvec, edge1, qvec);

    *v = glm_vec3_dot(direction, qvec) * inv_det;
    if (*v < 0.f || *v > 1.f) return FLT_MAX;

    t = glm_vec3_dot(edge2, qvec) * inv_det;
  }

  return t;
}
