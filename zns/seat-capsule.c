#include "seat-capsule.h"

#include <cglm/quat.h>
#include <cglm/vec3.h>
#include <math.h>
#include <zen-common.h>

#include "board.h"
#include "bounded.h"
#include "zen/appearance/board.h"
#include "zen/virtual-object.h"

#define INITIAL_BOARD_GAP (M_PI / 10.f)

void
zns_seat_capsule_rearrange(struct zns_seat_capsule *self)
{
  int board_length = wl_list_length(&self->board_list);
  int i = 0;
  float prev_azimuthal;
  struct zns_board *board;
  wl_list_for_each (board, &self->board_list, seat_capsule_link) {
    float azimuthal;
    if (i == 0) {
      azimuthal = M_PI / 2 + (board_length % 2 ? 0.f : INITIAL_BOARD_GAP);
    } else {
      azimuthal =
          prev_azimuthal + (i % 2 ? -1 : 1) * (2 * i * INITIAL_BOARD_GAP);
    }
    zns_seat_capsule_move_board(self, board, azimuthal, M_PI / 1.8f);
    prev_azimuthal = azimuthal;
    ++i;
  }
}

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
zns_seat_capsule_move_board(struct zns_seat_capsule *self,
    struct zns_board *board, float azimuthal, float polar)
{
  mat4 next_transform = GLM_MAT4_IDENTITY_INIT;

  board->seat_capsule_azimuthal = azimuthal;
  board->seat_capsule_polar = polar;

  if (M_PI_2 - self->flat_angle <= polar &&
      polar <= M_PI_2 + self->flat_angle) {
    vec3 position;
    float to_flat_surface = self->radius * cosf(self->flat_angle);
    position[0] = to_flat_surface;
    position[1] = tanf(M_PI_2 - polar) * to_flat_surface;
    position[2] = 0;

    glm_translate(next_transform, self->center);
    glm_rotate(next_transform, azimuthal, (vec3){0, 1, 0});
    glm_translate(next_transform, position);
    glm_rotate(next_transform, -M_PI_2, (vec3){0, 1, 0});

  } else {
    vec3 position;
    float orientation;

    position[0] = self->radius * sinf(polar);
    position[1] = self->radius * cosf(polar);
    position[2] = 0;

    orientation = M_PI - polar -
                  acosf(board->zn_board->geometry.size[1] / 2 / self->radius);

    glm_translate(next_transform, self->center);
    glm_rotate(next_transform, azimuthal, (vec3){0, 1, 0});
    glm_translate(next_transform, position);
    glm_rotate(next_transform, orientation, (vec3){0, 0, 1});
    glm_rotate(next_transform, -M_PI_2, (vec3){0, 1, 0});
  }

  zn_board_move(
      board->zn_board, board->zn_board->geometry.size, next_transform);
}

void
zns_seat_capsule_add_bounded(
    struct zns_seat_capsule *self, struct zns_bounded *bounded)
{
  wl_list_insert(&self->bounded_list, &bounded->seat_capsule_link);

  // TODO: calculate better initial position
  zns_seat_capsule_move_bounded(self, bounded, M_PI / 2.f, M_PI / 1.8f);
}

void
zns_seat_capsule_add_board(
    struct zns_seat_capsule *self, struct zns_board *board)
{
  wl_list_insert(&self->board_list, &board->seat_capsule_link);

  // TODO: calculate better initial position
  zns_seat_capsule_move_board(self, board, M_PI / 2.f, M_PI / 1.8f);

  zna_board_commit(board->zn_board->appearance);
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
  wl_list_init(&self->board_list);

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
  wl_list_remove(&self->board_list);
  free(self);
}
