#pragma once

#include <cglm/types.h>
#include <zgnr/virtual-object.h>

#include "zen/scene/scene.h"

struct zna_virtual_object;

struct zn_virtual_object {
  struct zgnr_virtual_object *zgnr_virtual_object;  // nonnull, reference
  struct wl_list link;                    // zn_scene::virtual_object_list
  struct zna_virtual_object *appearance;  // nonnull, owning

  vec3 position;
  versor quaternion;

  struct wl_listener zgnr_virtual_object_destroy_listener;
};

struct zn_virtual_object *zn_virtual_object_create(
    struct zgnr_virtual_object *zgnr_virtual_object, struct zn_scene *scene);
