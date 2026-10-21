#include "zen/config/config.h"

#include <string.h>
#include <tomlc17.h>

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
zn_config_create(const toml_datum_t *config_table)
{
  struct zn_config *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  zn_config_set_default(self);
  wl_array_init(&self->favorite_apps);

  if (!config_table) {
    zn_warn("Config is empty, use the default values.");
    return self;
  }

  // Strings in toml_datum_t are owned by toml_result_t
  toml_datum_t bg = toml_get(*config_table, "wallpaper");
  if (bg.type == TOML_TABLE) {
    toml_datum_t filepath = toml_get(bg, "filepath");
    if (filepath.type == TOML_STRING) {
      free(self->wallpaper_filepath);
      self->wallpaper_filepath = strdup(filepath.u.s);
    }
  }

  toml_datum_t board = toml_get(*config_table, "board");
  if (board.type == TOML_TABLE) {
    toml_datum_t initial_count = toml_get(board, "initial_count");
    if (initial_count.type == TOML_INT64) {
      self->board_initial_count = initial_count.u.int64;
      if (self->board_initial_count > BOARD_INITIAL_COUNT_MAX) {
        zn_warn(
            "The number of initial boards is limited to no more than %d, "
            "but requested %ld",
            BOARD_INITIAL_COUNT_MAX, self->board_initial_count);
        self->board_initial_count = BOARD_INITIAL_COUNT_MAX;
      }
    }
  }

  toml_datum_t space = toml_get(*config_table, "space");
  if (space.type == TOML_TABLE) {
    toml_datum_t default_app = toml_get(space, "default_app");
    if (default_app.type == TOML_STRING && strlen(default_app.u.s) != 0) {
      free(self->space_default_app);
      self->space_default_app = strdup(default_app.u.s);
    }
  }

  toml_datum_t favorite_app_entries = toml_get(*config_table, "favorite_apps");
  if (favorite_app_entries.type == TOML_ARRAY) {
    int entry_count = favorite_app_entries.u.arr.size;
    for (int i = 0; i < entry_count; i++) {
      toml_datum_t favorite_app_entry = favorite_app_entries.u.arr.elem[i];
      if (favorite_app_entry.type != TOML_TABLE) {
        zn_warn("Each 'favorite_apps' entry must be a table. Skipping");
        continue;
      }

      toml_datum_t exec = toml_get(favorite_app_entry, "exec");
      if (exec.type != TOML_STRING) {
        zn_warn(
            "Required config key not specified: 'favorite_apps:exec'. "
            "Skipping");
        continue;
      }

      struct zn_favorite_app *favorite_app =
          wl_array_add(&self->favorite_apps, sizeof(*favorite_app));

      favorite_app->exec = strdup(exec.u.s);

      toml_datum_t name = toml_get(favorite_app_entry, "name");
      if (name.type == TOML_STRING) {
        favorite_app->name = strdup(name.u.s);
      } else {
        favorite_app->name = strdup("");
      }

      toml_datum_t icon = toml_get(favorite_app_entry, "icon");
      if (icon.type == TOML_STRING) {
        favorite_app->icon = strdup(icon.u.s);
      } else {
        favorite_app->icon = strdup(UNKNOWN_APP_ICON);
      }

      toml_datum_t disable_2d = toml_get(favorite_app_entry, "disable_2d");
      if (disable_2d.type == TOML_BOOLEAN) {
        favorite_app->disable_2d = disable_2d.u.boolean;
      } else {
        favorite_app->disable_2d = false;
      }

      toml_datum_t disable_3d = toml_get(favorite_app_entry, "disable_3d");
      if (disable_3d.type == TOML_BOOLEAN) {
        favorite_app->disable_3d = disable_3d.u.boolean;
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
