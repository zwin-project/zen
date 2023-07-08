#pragma once

#include <stdbool.h>
#include <toml.h>
#include <wayland-util.h>

struct zn_config {
  toml_table_t *root_table;           // @nullable, @owning
  struct wl_array reserved_sections;  // [const char *]
};

/// @returns true if no other component has been reserved the section yet.
///
/// This doesn't check if the section exist or not
bool zn_config_reserve_section(
    struct zn_config *self, const char *section_name);

struct zn_config *zn_config_create(void);

void zn_config_destroy(struct zn_config *self);
