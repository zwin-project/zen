#pragma once

#include <cglm/cglm.h>
#include <wayland-server-core.h>

struct zn_ray {
  vec3 origin;

  struct {
    // angle with respect to y axis. 0 (+y) <= polar <= 2*pi (-y)
    float polar;
    // 0 <= azimuthal < 2*pi, (1, 0, 0) == 0, (0, 0, 1) == pi/2
    float azimuthal;
  } angle;  // radian

  struct {
    struct wl_signal destroy;
  } events;
};

struct zn_ray* zn_ray_create(void);

void zn_ray_destroy(struct zn_ray* self);
