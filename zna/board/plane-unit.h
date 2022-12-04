#pragma once

#include "base-unit.h"
#include "zen/board.h"

struct zna_board_plane_unit {
  struct zna_base_unit *base_unit;
};

void zna_board_plane_unit_commit(struct zna_board_plane_unit *self,
    struct zn_board *board, struct znr_virtual_object *virtual_object);

void zna_board_plane_unit_setup_renderer_objects(
    struct zna_board_plane_unit *self, struct znr_session *session,
    struct znr_virtual_object *virtual_object);

void zna_board_plane_unit_teardown_renderer_objects(
    struct zna_board_plane_unit *self);

struct zna_board_plane_unit *zna_board_plane_unit_create(
    struct zna_system *system);

void zna_board_plane_unit_destroy(struct zna_board_plane_unit *self);
