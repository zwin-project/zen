#pragma once

#include <zgnr/bounded.h>

#include "node.h"

struct zna_bounded;
struct zns_bounded_nameplate;

struct zns_bounded {
  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  struct zgnr_bounded *zgnr_bounded;  // nonnull

  struct zns_node *node;

  struct zns_bounded_nameplate *nameplate;

  float seat_capsule_azimuthal;
  float seat_capsule_polar;

  struct wl_list link;               // zn_shell::bounded_list
  struct wl_list seat_capsule_link;  // zns_seat_capsule::bounded_list

  struct zna_bounded *appearance;

  struct wl_listener zgnr_bounded_destroy_listener;
  struct wl_listener move_listener;
  struct wl_listener mapped_listener;
  struct wl_listener commit_listener;
};

struct zns_bounded *zns_bounded_create(struct zgnr_bounded *zgnr_bounded);
