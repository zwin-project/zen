#pragma once

#include <toml.h>

struct zn_config {
  char *wallpaper_filepath;  // can be empty string but cannot be null
};

struct zn_config *zn_config_create(struct toml_table_t *config_table);

void zn_config_destroy(struct zn_config *self);
