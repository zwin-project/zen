#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>

struct zna_ray;

struct zn_ray {
  vec3 origin;

  struct {
    // angle with respect to y axis. 0 (+y) <= polar <= pi (-y)
    float polar;
    // 0 <= azimuthal < 2*pi, (1, 0, 0) == 0, (0, 0, -1) == pi/2
    float azimuthal;
  } angle;  // radian

  struct {
    struct wl_signal destroy;  // (NULL)
    struct wl_signal motion;   // (NULL)
  } events;

  struct zna_ray* appearance;  // nonnull, owning
};

void zn_ray_get_tip(struct zn_ray* self, float length, vec3 tip);

void zn_ray_move(struct zn_ray* self, float polar, float azimuthal);

struct zn_ray* zn_ray_create(void);

void zn_ray_destroy(struct zn_ray* self);
