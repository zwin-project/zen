#include "zen/config/config.h"

#include <string.h>
#include <toml.h>

#include "zen-common/log.h"
#include "zen-common/util.h"

#define MAX_BOARD_INITIAL_COUNT 5
#define DEFAULT_BOARD_INITIAL_COUNT 0

static void
zn_config_set_default(struct zn_config *self)
{
  // It is freed in zen_config_destroy so DEFAULT_WALLPAPER
  // should not be passed directly.
  self->wallpaper_filepath = strdup(DEFAULT_WALLPAPER);
  self->board_initial_count = DEFAULT_BOARD_INITIAL_COUNT;
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
      if (self->board_initial_count > MAX_BOARD_INITIAL_COUNT) {
        zn_warn(
            "The number of the board is limited to less than %d, "
            "but requested %ld",
            MAX_BOARD_INITIAL_COUNT, self->board_initial_count);
        self->board_initial_count = MAX_BOARD_INITIAL_COUNT;
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
  free(self->wallpaper_filepath);
  free(self);
}
