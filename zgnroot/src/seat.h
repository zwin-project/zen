#pragma once

#include "zgnr/seat.h"

struct zgnr_seat_impl {
  struct zgnr_seat base;
  struct wl_global* global;

  struct wl_list resource_list;
};
