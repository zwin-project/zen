#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>
#include <zigen-protocol.h>

struct zgnr_virtual_object;

struct zgnr_seat {
  void *user_data;
  uint32_t capabilities;
};

void zgnr_seat_set_capabilities(struct zgnr_seat *self, uint32_t capabilities);

void zgnr_seat_send_ray_enter(struct zgnr_seat *self,
    struct zgnr_virtual_object *virtual_object, uint32_t serial, vec3 origin,
    vec3 direction);

void zgnr_seat_send_ray_motion(struct zgnr_seat *self, struct wl_client *client,
    uint32_t time_msec, vec3 origin, vec3 direction);

void zgnr_seat_send_ray_leave(struct zgnr_seat *self,
    struct zgnr_virtual_object *virtual_object, uint32_t serial);

void zgnr_seat_send_ray_button(struct zgnr_seat *self, struct wl_client *client,
    uint32_t serial, uint32_t time_msec, uint32_t button,
    enum zgn_ray_button_state state);

struct zgnr_seat *zgnr_seat_create(struct wl_display *display);

void zgnr_seat_destroy(struct zgnr_seat *self);
