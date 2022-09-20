#pragma once

#include <wlr/types/wlr_output.h>

#include "zen/server.h"

/** this destroys itself when the given wlr_output is destroyed */
struct zn_output {
  struct wlr_output *wlr_output;  // nonnull
  struct zn_server *server;       // nonnull

  /** nonnull, automatically destroyed when wlr_output is destroyed */
  struct wlr_output_damage *damage;

  struct zn_screen *screen;  // controlled by zn_output

  // TODO: use this for better repaint loop
  struct wl_event_source *repaint_timer;

  struct wl_listener wlr_output_destroy_listener;
  struct wl_listener damage_frame_listener;
  enum wl_output_transform output_transform;
  double output_scale;
};

/**
 * @param fbox in effective coordinate
 */
void zn_output_add_damage_box(
    struct zn_output *self, struct wlr_fbox *effective_box);

void zn_output_add_damage_whole(struct zn_output *self);

void zn_output_box_effective_to_transformed_coords(struct zn_output *self,
    struct wlr_fbox *effective, struct wlr_box *transformed);

void zn_output_update_global(struct zn_output *self);

struct zn_output *zn_output_create(
    struct wlr_output *wlr_output, struct zn_server *server);
