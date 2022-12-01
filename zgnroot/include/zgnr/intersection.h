#pragma once

#include <cglm/types.h>

/**
 * @return the distance from the origin to the intersection, or FLT_MAX if not
 * intersecting
 */
float zgnr_intersection_ray_obb(
    vec3 origin, vec3 direction, vec3 aabb_half_size, mat4 transform);

/**
 * @return the distance from the origin to the intersection, or FLT_MAX if not
 * intersecting
 */
float zgnr_intersection_ray_sphere(
    vec3 origin, vec3 direction, vec3 center, float radius);
