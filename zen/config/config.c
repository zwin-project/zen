#include "zen/config/config.h"

#include <string.h>
#include <toml.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/favorite-app.h"

#define BOARD_INITIAL_COUNT_MAX 5
#define BOARD_INITIAL_COUNT_DEFAULT 0
#define SPACE_DEFAULT_APP_DEFAULT "zennist"

static void
zn_config_set_default(struct zn_config *self)
{
  // It is freed in zen_config_destroy so default value
  // should not be passed directly.
  self->space_default_app = strdup(SPACE_DEFAULT_APP_DEFAULT);
  self->wallpaper_filepath = strdup(DEFAULT_WALLPAPER);
  self->board_initial_count = BOARD_INITIAL_COUNT_DEFAULT;
}

struct zn_config *
zn_config_create(struct toml_table_t *config_table)
{
  struct zn_config *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  zn_config_set_default(self);

  if (!config_table) {
    zn_warn("Config is empty, use the default values.");
    return self;
  }

  toml_table_t *bg = toml_table_in(config_table, "wallpaper");
  if (bg != NULL) {
    toml_datum_t filepath = toml_string_in(bg, "filepath");
    free(self->wallpaper_filepath);
    self->wallpaper_filepath = filepath.u.s;
  }

  toml_table_t *board = toml_table_in(config_table, "board");
  if (board != NULL) {
    toml_datum_t initial_count = toml_int_in(board, "initial_count");
    if (initial_count.ok) {
      self->board_initial_count = initial_count.u.i;
      if (self->board_initial_count > BOARD_INITIAL_COUNT_MAX) {
        zn_warn(
            "The number of initial boards is limited to no more than %d, "
            "but requested %ld",
            BOARD_INITIAL_COUNT_MAX, self->board_initial_count);
        self->board_initial_count = BOARD_INITIAL_COUNT_MAX;
      }
    }
  }

  toml_table_t *space = toml_table_in(config_table, "space");
  if (space != NULL) {
    toml_datum_t default_app = toml_string_in(space, "default_app");
    if (default_app.ok && strlen(default_app.u.s) != 0) {
      self->space_default_app = strdup(default_app.u.s);
    }
  }

  wl_array_init(&self->favorite_apps);
  toml_array_t *favorite_app_entries =
      toml_array_in(config_table, "favorite_apps");
  if (favorite_app_entries != NULL) {
    int entry_count = toml_array_nelem(favorite_app_entries);
    for (int i = 0; i < entry_count; i++) {
      toml_table_t *favorite_app_entry = toml_table_at(favorite_app_entries, i);

      toml_datum_t exec = toml_string_in(favorite_app_entry, "exec");
      if (!exec.ok) {
        zn_warn(
            "Required config key not specified: 'favorite_apps:exec'. "
            "Skipping");
        continue;
      }

      struct zn_favorite_app *favorite_app =
          wl_array_add(&self->favorite_apps, sizeof(*favorite_app));

      favorite_app->exec = exec.u.s;

      toml_datum_t name = toml_string_in(favorite_app_entry, "name");
      if (name.ok) {
        favorite_app->name = name.u.s;
      } else {
        favorite_app->name = strdup("");
      }

      toml_datum_t icon = toml_string_in(favorite_app_entry, "icon");
      if (icon.ok) {
        favorite_app->icon = icon.u.s;
      } else {
        favorite_app->icon = strdup(UNKNOWN_APP_ICON);
      }

      toml_datum_t disable_2d = toml_bool_in(favorite_app_entry, "disable_2d");
      if (disable_2d.ok) {
        favorite_app->disable_2d = disable_2d.u.b;
      } else {
        favorite_app->disable_2d = false;
      }

      toml_datum_t disable_3d = toml_bool_in(favorite_app_entry, "disable_3d");
      if (disable_3d.ok) {
        favorite_app->disable_3d = disable_3d.u.b;
      } else {
        favorite_app->disable_3d = false;
      }
    }
  }

  return self;

err:
  return NULL;
}

void
zn_config_destroy(struct zn_config *self)
{
  struct zn_favorite_app *favorite_app;
  wl_array_for_each (favorite_app, &self->favorite_apps) {
    free(favorite_app->exec);
    free(favorite_app->name);
    free(favorite_app->icon);
  }

  wl_array_release(&self->favorite_apps);

  free(self->space_default_app);
  free(self->wallpaper_filepath);
  free(self);
}
