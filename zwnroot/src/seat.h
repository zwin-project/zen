#pragma once

#include "zwnr/seat.h"

struct zwnr_seat_impl {
  struct zwnr_seat base;
  struct wl_global *global;
  struct wl_display *display;

  struct wl_list resource_list;

  struct wl_list seat_ray_list;  // zwnr_seat_ray::link

  struct wl_listener ray_focus_virtual_object_listener;
};
