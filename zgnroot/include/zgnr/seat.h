#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>

struct zgnr_virtual_object;

struct zgnr_seat {
  void* user_data;
  uint32_t capabilities;
};

void zgnr_seat_set_capabilities(struct zgnr_seat* self, uint32_t capabilities);

void zgnr_seat_send_ray_enter(struct zgnr_seat* self,
    struct zgnr_virtual_object* virtual_object, vec3 origin, vec3 direction);

void zgnr_seat_send_ray_motion(struct zgnr_seat* self, struct wl_client* client,
    uint32_t time_msec, vec3 origin, vec3 direction);

void zgnr_seat_send_ray_leave(
    struct zgnr_seat* self, struct zgnr_virtual_object* virtual_object);

struct zgnr_seat* zgnr_seat_create(struct wl_display* display);

void zgnr_seat_destroy(struct zgnr_seat* self);
