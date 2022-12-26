#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>
#include <zigen-protocol.h>

struct zgnr_virtual_object;

struct zgnr_seat {
  void *user_data;
  uint32_t capabilities;

  struct {
    struct zgnr_virtual_object *focus_virtual_object;
    uint32_t last_button_serial;
    bool sent_axis_source;
  } ray_state;
};

void zgnr_seat_set_capabilities(struct zgnr_seat *self, uint32_t capabilities);

void zgnr_seat_ray_enter(struct zgnr_seat *self,
    struct zgnr_virtual_object *virtual_object, vec3 origin, vec3 direction);

void zgnr_seat_ray_send_motion(
    struct zgnr_seat *self, uint32_t time_msec, vec3 origin, vec3 direction);

void zgnr_seat_ray_clear_focus(struct zgnr_seat *self);

void zgnr_seat_ray_send_button(struct zgnr_seat *self, uint32_t time_msec,
    uint32_t button, enum zgn_ray_button_state state);

void zgnr_seat_ray_send_axis(struct zgnr_seat *self, uint32_t time_msec,
    enum zgn_ray_axis axis, double value, int32_t value_discrete,
    enum zgn_ray_axis_source source);

void zgnr_seat_ray_send_frame(struct zgnr_seat *self);

struct zgnr_seat *zgnr_seat_create(struct wl_display *display);

void zgnr_seat_destroy(struct zgnr_seat *self);
