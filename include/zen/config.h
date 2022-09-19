#pragma once

struct zn_config {
  char* bg_image_file;
};

struct zn_config* zn_config_create(void);
void zn_config_destroy(struct zn_config* self);