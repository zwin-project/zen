#pragma once

#include <toml.h>
struct zn_favorite_app;

struct zn_config {
  char *space_default_app;   // nonnull, non-empty
  char *wallpaper_filepath;  // can be empty string but cannot be null
  int64_t board_initial_count;
  int64_t num_favorite_apps;
  struct zn_favorite_app *favorite_apps;
};

struct zn_config *zn_config_create(struct toml_table_t *config_table);

void zn_config_destroy(struct zn_config *self);
