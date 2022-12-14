#include "seat-capsule.h"

#include <cglm/quat.h>
#include <cglm/vec3.h>
#include <zen-common.h>

#include "bounded.h"
#include "zen/virtual-object.h"

void
zns_seat_capsule_move_bounded(struct zns_seat_capsule *self,
    struct zns_bounded *bounded, float azimuthal, float polar)
{
  struct zn_virtual_object *zn_virtual_object =
      bounded->zgnr_bounded->virtual_object->user_data;

  bounded->seat_capsule_azimuthal = azimuthal;
  bounded->seat_capsule_polar = polar;

  versor quaternion;
  vec3 position;

  if (M_PI_2 - self->flat_angle <= polar &&
      polar <= M_PI_2 + self->flat_angle) {
    float to_flat_surface = self->radius * cosf(self->flat_angle);
    float to_bounded_center =
        to_flat_surface + bounded->zgnr_bounded->current.half_size[2];
    glm_quat(quaternion, azimuthal - M_PI_2, 0, 1, 0);
    position[0] = cosf(azimuthal) * to_bounded_center;
    position[1] = tanf(M_PI_2 - polar) * to_flat_surface +
                  bounded->zgnr_bounded->current.half_size[1];
    position[2] = -sin(azimuthal) * to_bounded_center;
    glm_vec3_add(position, self->center, position);
  } else {
    float l = self->radius * sinf(polar) +
              bounded->zgnr_bounded->current.half_size[2];
    glm_quat(quaternion, azimuthal - M_PI_2, 0, 1, 0);
    position[0] = cosf(azimuthal) * l;
    position[1] = cosf(polar) * self->radius +
                  bounded->zgnr_bounded->current.half_size[1];
    position[2] = -sinf(azimuthal) * l;
    glm_vec3_add(position, self->center, position);
  }

  zn_virtual_object_move(zn_virtual_object, position, quaternion);
}

void
zns_seat_capsule_add_bounded(
    struct zns_seat_capsule *self, struct zns_bounded *bounded)
{
  wl_list_insert(&self->bounded_list, &bounded->seat_capsule_link);

  // TODO: calculate better initial position
  zns_seat_capsule_move_bounded(self, bounded, M_PI / 2.f, M_PI / 1.8f);
}

struct zns_seat_capsule *
zns_seat_capsule_create(void)
{
  struct zns_seat_capsule *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wl_list_init(&self->bounded_list);

  glm_vec3_copy((vec3){0, 0.8f, 0}, self->center);
  self->radius = 1;
  self->flat_angle = M_PI / 6.f;

  return self;

err:
  return NULL;
}

void
zns_seat_capsule_destroy(struct zns_seat_capsule *self)
{
  wl_list_remove(&self->bounded_list);
  free(self);
}
