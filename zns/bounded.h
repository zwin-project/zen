#pragma once

#include <zgnr/bounded.h>

struct zns_bounded {
  struct zgnr_bounded* zgnr_bounded;  // nonnull

  struct wl_listener zgnr_bounded_destroy_listener;

  struct wl_list link;  // zn_shell::bounded_list
};

struct zns_bounded* zns_bounded_create(struct zgnr_bounded* zgnr_bounded);
