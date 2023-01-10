#pragma once

#include <wlr/util/box.h>

struct zn_screen;
struct zn_board;

struct zn_screen_interface {
  /**
   * @param box : effective coordinate system
   */
  void (*damage)(void *user_data, struct wlr_fbox *box);
  void (*damage_whole)(void *user_data);
  void (*get_effective_size)(void *user_data, double *width, double *height);
};

struct zn_screen {
  void *user_data;
  const struct zn_screen_interface *implementation;

  double x, y;  // layout coordinate, controlled by zn_screen_layout

  struct wl_list link;  // zn_screen_layout::screen_list

  // inserted by zn_board
  struct wl_list board_list;  // zn_board::screen_link

  // nonnull when screen display system and mapped to zn_scene
  // if not null, this->current_board->screen == this
  struct zn_board *current_board;

  struct wl_listener current_board_destroy_listener;

  struct zn_zigzag_layout *zn_zigzag_layout;  // nonnull, owning

  struct {
    struct wl_signal current_board_changed;  // (struct zn_board *)
    struct wl_signal destroy;                // (NULL)
  } events;
};

void zn_screen_damage_force(struct zn_screen *self, struct wlr_fbox *box);

/**
 * @param box : effective coordinate system
 */
void zn_screen_damage(struct zn_screen *self, struct wlr_fbox *box);

void zn_screen_damage_whole(struct zn_screen *self);

void zn_screen_send_frame_done(struct zn_screen *self, struct timespec *when);

/**
 * Convert screen local effective coordinates to layout coordinates
 * @param x screen local effective coordinates
 * @param y screen local effective coordinates
 * @param dest_x layout coordinates
 * @param dest_y layout coordinates
 */
void zn_screen_get_screen_layout_coords(
    struct zn_screen *self, double x, double y, double *dest_x, double *dest_y);

void zn_screen_get_effective_size(
    struct zn_screen *self, double *width, double *height);

void zn_screen_set_current_board(
    struct zn_screen *self, struct zn_board *board);

void zn_screen_switch_to_next_board(struct zn_screen *self);

void zn_screen_switch_to_prev_board(struct zn_screen *self);

struct zn_screen *zn_screen_create(
    const struct zn_screen_interface *implementation, void *user_data);

void zn_screen_destroy(struct zn_screen *self);
