#pragma once

#include <cglm/types.h>
#include <stdbool.h>

struct zns_bounded;

/**
 * The life time of given zns_bounded must be longer than this object
 */
struct zns_bounded_nameplate {
  struct zns_bounded *bounded;  // nonnull

  struct zns_node *node;

  bool has_ray_focus;

  struct {
    float width;
    float height;
  } geometry;
};

/**
 * returns transformation matrix relative to the world space
 */
void zns_bounded_nameplate_get_transform(
    struct zns_bounded_nameplate *self, mat4 transform);

struct zns_bounded_nameplate *zns_bounded_nameplate_create(
    struct zns_bounded *bounded);

void zns_bounded_nameplate_destroy(struct zns_bounded_nameplate *self);
