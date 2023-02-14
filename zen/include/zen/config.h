#pragma once

#include <stdbool.h>
#include <toml.h>
#include <wayland-util.h>

struct zn_config_section {
  /// This must be called once in zn_config_add_section
  /// @param table is nullable
  /// @return true if succeeded, false otherwise
  bool (*load)(struct zn_config_section *self, toml_table_t *table);

  /// This must be called once when zn_config_destroy
  void (*destroy)(struct zn_config_section *self);
};

struct zn_config_section_info {
  char *name;                         // @nonnull, @owning
  struct zn_config_section *section;  // @nonnull, @owning
};

struct zn_config {
  toml_table_t *root_table;  // @nullable, @owning
  struct wl_array sections;  // [struct zn_config_section_info]
};

/// @returns true if succeeded, false otherwise
/// @param section's ownership will be moved to the zn_config
bool zn_config_add_section(struct zn_config *self, const char *name,
    struct zn_config_section *section);

/// @return value is nullable
struct zn_config_section *zn_config_get_section(
    struct zn_config *self, const char *name);

struct zn_config *zn_config_create(void);

void zn_config_destroy(struct zn_config *self);
