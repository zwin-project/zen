#pragma once

#include <cglm/types.h>
#include <zwnr/virtual-object.h>

struct zna_virtual_object;
struct zn_scene;

struct zn_virtual_object {
  struct zwnr_virtual_object *zwnr_virtual_object;  // nonnull, reference
  struct wl_list link;                    // zn_scene::virtual_object_list
  struct zna_virtual_object *appearance;  // nonnull, owning

  vec3 position;
  versor quaternion;
  mat4 model_matrix;  // the combination of `position` and `quaternion`
  mat4 model_invert;

  struct {
    struct wl_signal move;  // (NULL)
  } events;

  struct wl_listener zwnr_virtual_object_destroy_listener;
};

void zn_virtual_object_move(
    struct zn_virtual_object *self, vec3 position, versor quaternion);

struct zn_virtual_object *zn_virtual_object_create(
    struct zwnr_virtual_object *zwnr_virtual_object);
