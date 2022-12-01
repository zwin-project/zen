#pragma once

#include "zen/appearance/system.h"
#include "zen/board.h"

struct zna_board;

struct zna_board *zna_board_create(
    struct zn_board *board, struct zna_system *system);

void zna_board_destroy(struct zna_board *self);
