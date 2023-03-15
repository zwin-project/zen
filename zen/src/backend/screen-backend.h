#pragma once

#include <wlr/types/wlr_layer_shell_v1.h>

struct zn_snode;
struct zn_layer_surface;

struct zn_screen_backend {
  struct zn_output *output;  // @nonnull, @outlive

  // Every child snode in each layer snode must be that of zn_layer_surface
  // Index is corresponding to enum zwlr_layer_shell_v1_layer
  // 0: background, 1: button, 2: top, 3: overlay
  struct zn_snode *layers[4];  // each layer is @nonnull, @owning
};

void zn_screen_backend_add_layer_surface(struct zn_screen_backend *self,
    struct zn_layer_surface *layer_surface,
    enum zwlr_layer_shell_v1_layer layer);

struct zn_snode *zn_screen_backend_get_layer(
    struct zn_screen_backend *self, enum zwlr_layer_shell_v1_layer layer);

struct zn_screen_backend *zn_screen_backend_create(struct zn_output *output);

void zn_screen_backend_destroy(struct zn_screen_backend *self);
