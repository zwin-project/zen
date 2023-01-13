#pragma once

#include "virtual-object.h"
#include "zgnr/bounded.h"

/**
 * When the associated virtual object is destroyed, this object is destroyed
 * and the wl_resource becomes inert (resource::data == NULL).
 */
struct zgnr_bounded_impl {
  struct zgnr_bounded base;

  struct wl_listener virtual_object_destroy_listener;
  struct wl_listener virtual_object_commit_listener;

  struct wl_list configure_list;  // zgnr_bounded_configure::link

  struct {
    vec3 half_size;
    struct zgnr_region_node *region;  // nullable
    char *title;                      // nonnull, null-terminated, can be empty
    uint32_t damage;                  // bit sum of enum zgnr_bounded_damage
  } pending;

  bool configured;
  bool mapped;

  struct wl_resource *resource;
};

void zgnr_bounded_configure(struct zgnr_bounded_impl *self, vec3 half_size);

struct zgnr_bounded_impl *zgnr_bounded_create(struct wl_client *client,
    uint32_t id, struct zgnr_virtual_object_impl *virtual_object);
