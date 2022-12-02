#pragma once

#include <cglm/types.h>
#include <stdbool.h>

/**
 * @param transform : translation and rotation only
 * @return the distance from the origin to the intersection, or FLT_MAX if not
 * intersecting
 */
float zgnr_intersection_ray_obb(
    vec3 origin, vec3 direction, vec3 aabb_half_size, mat4 transform);

//     v2--------------
//     /              /
//    v    front     /
//   /              /
// v0-------u------v1

/**
 * @param u
 * @param v intersection point P = V0 + u*(v1 - v0) + v*(v2 - v0)
 * @param test_cull : if true, detect intersection only when the ray comes from
 * front face side.
 * @return the distance from the origin to the intersection, or FLT_MAX if not
 * intersecting
 */
float zgnr_intersection_ray_parallelogram(vec3 origin, vec3 direction, vec3 v0,
    vec3 v1, vec3 v2, float *u, float *v, bool test_cull);
