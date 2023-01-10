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
  self->num_favorite_apps = 0;
  self->favorite_apps = NULL;
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

  toml_array_t *favorite_apps = toml_array_in(config_table, "favorite-apps");
  if (favorite_apps != NULL) {
    self->num_favorite_apps = toml_array_nelem(favorite_apps);
    self->favorite_apps =
        zalloc((sizeof *self->favorite_apps) * self->num_favorite_apps);
    for (int i = 0; i < self->num_favorite_apps; i++) {
      toml_table_t *favorite_app = toml_table_at(favorite_apps, i);

      toml_datum_t name = toml_string_in(favorite_app, "name");
      if (name.ok) {
        self->favorite_apps[i].name = name.u.s;
      }

      toml_datum_t exec = toml_string_in(favorite_app, "exec");
      if (exec.ok && strlen(exec.u.s) != 0) {
        self->favorite_apps[i].exec = exec.u.s;
      } else {
        zn_error(
            "Required config key not specified: "
            "favorite-apps:exec");
        goto err_favorite_apps;
      }

      toml_datum_t icon = toml_string_in(favorite_app, "icon");
      if (icon.ok && strlen(icon.u.s) != 0) {
        self->favorite_apps[i].icon = icon.u.s;
      } else {
        zn_error(
            "Required config key not specified: "
            "favorite-apps:icon");
        goto err_favorite_apps;
      }

      toml_datum_t disable_2d = toml_bool_in(favorite_app, "disable_2d");
      if (disable_2d.ok) {
        self->favorite_apps[i].disable_2d = disable_2d.u.b;
      } else {
        self->favorite_apps[i].disable_2d = false;
      }

      toml_datum_t disable_3d = toml_bool_in(favorite_app, "disable_3d");
      if (disable_3d.ok) {
        self->favorite_apps[i].disable_3d = disable_3d.u.b;
      } else {
        self->favorite_apps[i].disable_3d = false;
      }
    }
  };

  return self;

err_favorite_apps:
  free(self->favorite_apps);

err:
  return NULL;
}

void
zn_config_destroy(struct zn_config *self)
{
  if (self->favorite_apps != NULL) free(self->favorite_apps);
  free(self->space_default_app);
  free(self->wallpaper_filepath);
  free(self);
}
