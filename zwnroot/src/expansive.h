#pragma once

#include "virtual-object.h"
#include "zwnr/expansive.h"

enum zwnr_expansive_damage {
  ZWNR_EXPANSIVE_DAMAGE_REGION = 1 << 0,
};

/**
 * When the associated virtual object is destroyed, this object is destroyed
 * and the wl_resource becomes inert (resource::data == NULL).
 */
struct zwnr_expansive_impl {
  struct zwnr_expansive base;

  struct wl_resource *resource;

  struct wl_listener virtual_object_destroy_listener;
  struct wl_listener virtual_object_commit_listener;

  struct {
    struct zwnr_region_node *region;  // nullable
    uint32_t damage;                  // bit sum of enum zwnr_expansive_damage
  } pending;
};

struct zwnr_expansive_impl *zwnr_expansive_create(struct wl_client *client,
    uint32_t id, struct zwnr_virtual_object_impl *virtual_object);
