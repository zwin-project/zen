#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>
#include <zigen-protocol.h>

struct zna_ray;
struct zn_ray;
struct zn_ray_grab;
struct zn_virtual_object;

struct zn_ray_grab_interface {
  void (*motion_relative)(struct zn_ray_grab* grab, vec3 origin, float polar,
      float azimuthal, uint32_t time_msec);
  void (*button)(struct zn_ray_grab* grab, uint32_t time_msec, uint32_t button,
      enum zgn_ray_button_state state);
  void (*cancel)(struct zn_ray_grab* grab);
};

struct zn_ray_grab {
  const struct zn_ray_grab_interface* interface;
  struct zn_ray* ray;
};

struct zn_ray {
  struct zn_ray_grab* grab;  // nonnull

  vec3 origin;  // modified by grab

  struct {
    // angle with respect to y axis. 0 (+y) <= polar <= pi (-y)
    float polar;
    // 0 <= azimuthal < 2*pi, (1, 0, 0) == 0, (0, 0, -1) == pi/2
    float azimuthal;
  } angle;  // radian, modified by grab

  vec3 direction;  // consistent with the values of angle, length == 1

  float length;

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  struct zna_ray* appearance;  // nonnull, owning
};

/**
 * @param tip returns the tip position of the ray
 */
void zn_ray_get_tip(struct zn_ray* self, vec3 tip);

/**
 * Returns the origin and direction of ray in the virtual object's local
 * coordinate system.
 */
void zn_ray_get_local_origin_direction(struct zn_ray* self,
    struct zn_virtual_object* virtual_object, vec3 local_origin,
    vec3 local_direction);

void zn_ray_set_length(struct zn_ray* self, float length);

void zn_ray_move(
    struct zn_ray* self, vec3 origin, float polar, float azimuthal);

void zn_ray_start_grab(struct zn_ray* ray, struct zn_ray_grab* grab);

void zn_ray_end_grab(struct zn_ray* ray);

struct zn_ray* zn_ray_create(void);

void zn_ray_destroy(struct zn_ray* self);
