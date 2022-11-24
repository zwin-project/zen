#include "default.h"

#include <cglm/vec3.h>
#include <zen-common.h>

static void
default_grab_motion_relative(
    struct zn_ray_grab* grab, vec3 origin, float polar, float azimuthal)
{
  float next_polar = grab->ray->angle.polar + polar;
  if (next_polar < 0)
    next_polar = 0;
  else if (next_polar > M_PI)
    next_polar = M_PI;

  float next_azimuthal = grab->ray->angle.azimuthal + azimuthal;
  while (next_azimuthal >= 2 * M_PI) next_azimuthal -= 2 * M_PI;
  while (next_azimuthal < 0) next_azimuthal += 2 * M_PI;

  vec3 next_origin;
  glm_vec3_add(grab->ray->origin, origin, next_origin);

  zn_ray_move(grab->ray, next_origin, next_polar, next_azimuthal);
}

static void
default_grab_cancel(struct zn_ray_grab* grab)
{
  UNUSED(grab);
}

static const struct zn_ray_grab_interface implementation = {
    .motion_relative = default_grab_motion_relative,
    .cancel = default_grab_cancel,
};

void
zns_default_ray_grab_init(struct zns_default_ray_grab* self)
{
  self->base.interface = &implementation;
}

void
zns_default_ray_grab_fini(struct zns_default_ray_grab* self)
{
  UNUSED(self);
}
