#pragma once

#include <zgnr/expansive.h>

struct zns_expansive {
  struct zgnr_expansive *zgnr_expansive;  // nonnull

  struct wl_listener zgnr_expansive_destroy_listener;
};

struct zns_expansive *zns_expansive_create(
    struct zgnr_expansive *zgnr_expansive);
