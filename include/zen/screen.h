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

  double x, y;

  struct wl_list link;  // zn_screen_layout::screens

  // nonnull when screen display system and mapped to zn_scene
  // controlled by zn_scene, if not null, this->board->screen == this
  struct zn_board *board;

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;
};

/**
 * @param box : effective coordinate system
 */
void zn_screen_damage(struct zn_screen *self, struct wlr_fbox *box);

void zn_screen_damage_whole(struct zn_screen *self);

void zn_screen_get_effective_size(
    struct zn_screen *self, double *width, double *height);

struct zn_screen *zn_screen_create(
    const struct zn_screen_interface *implementation, void *user_data);

void zn_screen_destroy(struct zn_screen *self);
