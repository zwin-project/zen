#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>

struct zns_bounded;
struct zns_board;

struct zns_seat_capsule {
  vec3 center;
  float radius;
  float flat_angle;  // radian

  struct wl_list bounded_list;  // zns_bounded::seat_capsule_link
  struct wl_list board_list;    // zns_board::seat_capsule_link
};

void zns_seat_capsule_move_bounded(struct zns_seat_capsule *self,
    struct zns_bounded *bounded, float azimuthal, float polar);

void zns_seat_capsule_move_board(struct zns_seat_capsule *self,
    struct zns_board *board, float azimuthal, float polar);

void zns_seat_capsule_add_bounded(
    struct zns_seat_capsule *self, struct zns_bounded *bounded);

void zns_seat_capsule_add_board(
    struct zns_seat_capsule *self, struct zns_board *board);

struct zns_seat_capsule *zns_seat_capsule_create(void);

void zns_seat_capsule_destroy(struct zns_seat_capsule *self);
