#ifndef ZEN_SHELL_INTERSECTION_H
#define ZEN_SHELL_INTERSECTION_H

#include <cglm/cglm.h>

float zen_shell_ray_obb_intersection(
    vec3 ray_origin, vec3 ray_direction, vec3 aabb_half_size, mat4 transform);

#endif  //  ZEN_SHELL_INTERSECTION_H
