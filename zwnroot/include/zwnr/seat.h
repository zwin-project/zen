#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>
#include <zwin-protocol.h>

struct zwnr_virtual_object;

struct zwnr_seat {
  void *user_data;
  uint32_t capabilities;

  struct {
    struct zwnr_virtual_object *focus_virtual_object;
    uint32_t last_button_serial;
    bool sent_axis_source;
  } ray_state;
};

void zwnr_seat_set_capabilities(struct zwnr_seat *self, uint32_t capabilities);

void zwnr_seat_ray_enter(struct zwnr_seat *self,
    struct zwnr_virtual_object *virtual_object, vec3 origin, vec3 direction);

void zwnr_seat_ray_send_motion(
    struct zwnr_seat *self, uint32_t time_msec, vec3 origin, vec3 direction);

void zwnr_seat_ray_clear_focus(struct zwnr_seat *self);

void zwnr_seat_ray_send_button(struct zwnr_seat *self, uint32_t time_msec,
    uint32_t button, enum zwn_ray_button_state state);

void zwnr_seat_ray_send_axis(struct zwnr_seat *self, uint32_t time_msec,
    enum zwn_ray_axis axis, double value, int32_t value_discrete,
    enum zwn_ray_axis_source source);

void zwnr_seat_ray_send_frame(struct zwnr_seat *self);

struct zwnr_seat *zwnr_seat_create(struct wl_display *display);

void zwnr_seat_destroy(struct zwnr_seat *self);
