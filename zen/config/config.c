#include "zen/config/config.h"

#include <string.h>
#include <toml.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/config/autostart.h"

#define BOARD_INITIAL_COUNT_MAX 5
#define BOARD_INITIAL_COUNT_DEFAULT 0

static void
zn_config_set_default(struct zn_config *self)
{
  wl_list_init(&self->autostart_list);
  // It is freed in zen_config_destroy so DEFAULT_WALLPAPER
  // should not be passed directly.
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

  toml_table_t *general = toml_table_in(config_table, "general");
  if (general != NULL) {
    toml_array_t *autostarts = toml_array_in(general, "autostart");
    if (autostarts) {
      for (int i = 0;; ++i) {
        toml_datum_t command = toml_string_at(autostarts, i);
        if (!command.ok) {
          break;
        }
        struct zn_autostart *autostart = zn_autostart_create(command.u.s);
        if (autostart) {
          wl_list_insert(&self->autostart_list, &autostart->link);
        }
      }
    }
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

  return self;

err:
  return NULL;
}

void
zn_config_destroy(struct zn_config *self)
{
  struct zn_autostart *autostart, *tmp;
  wl_list_for_each_safe (autostart, tmp, &self->autostart_list, link) {
    zn_autostart_destroy(autostart);
  }
  free(self->wallpaper_filepath);
  free(self);
}
