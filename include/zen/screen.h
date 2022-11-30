#pragma once

#include <wlr/util/box.h>

struct zn_screen;
struct zn_board;

struct zn_screen_interface {
  /**
   * @param box : effective coordinate system
   */
  void (*damage)(void *user_data, struct wlr_fbox *box);
};

struct zn_screen {
  void *user_data;
  const struct zn_screen_interface *implementation;

  struct wl_list link;  // zn_scene::screen_list; used by zn_scene

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

struct zn_screen *zn_screen_create(
    const struct zn_screen_interface *implementation, void *user_data);

void zn_screen_destroy(struct zn_screen *self);
