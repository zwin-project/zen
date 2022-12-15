#pragma once

#include "board/plane-unit.h"
#include "zen/appearance/board.h"

struct zna_board {
  struct zn_board *zn_board;  // nonnull
  struct zna_system *system;  // nonnull

  // null when the current session does not exist, not null otherwise
  struct znr_virtual_object *virtual_object;

  struct zna_board_plane_unit *plane_unit;  // nonnull

  struct wl_listener session_created_listener;
  struct wl_listener session_destroyed_listener;
  struct wl_listener session_frame_listener;
};
