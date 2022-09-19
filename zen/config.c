#include "zen/config.h"

#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <toml.h>
#include <unistd.h>

#include "build-config.h"
#include "zen-common/log.h"
#include "zen-common/util.h"

static struct toml_table_t *
zn_config_get_toml_table(void)
{
  char *config_home;
  config_home = getenv("XDG_CONFIG_HOME");
  if (config_home == NULL) {
    char *homedir = getenv("HOME");
    if (homedir == NULL) homedir = getpwuid(getuid())->pw_dir;
    if (homedir == NULL) {
      zn_warn("Could not find the home directory");
      return NULL;
    }
    config_home = strcat(homedir, "/.config");
  }

  char *config_filename = strcat(config_home, "/zen-desktop/config.toml");
  char errbuf[200];
  FILE *fp = fopen(config_filename, "r");
  if (fp == NULL) {
    zn_warn("config.toml not found in %s\n", config_filename);
    return NULL;
  }
  toml_table_t *tbl = toml_parse_file(fp, errbuf, sizeof(errbuf));
  fclose(fp);

  if (tbl == NULL) zn_warn("Failed to parse config.toml: %s\n", errbuf);
  return tbl;
}

static void
zn_config_set_default(struct zn_config *self)
{
  self->bg_image_file = DEFAULT_WALLPAPER;
}

struct zn_config *
zn_config_create(void)
{
  struct zn_config *self;
  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }
  zn_config_set_default(self);
  toml_table_t *tbl = zn_config_get_toml_table();
  if (!tbl) {
    zn_warn("Could not get the toml table");
    return self;
  }
  toml_table_t *bg = toml_table_in(tbl, "background");
  if (bg != NULL) {
    toml_datum_t image = toml_string_in(bg, "image");
    self->bg_image_file = image.u.s;
  }
  toml_free(tbl);
  return self;
err:
  return NULL;
}
void
zn_config_destroy(struct zn_config *self)
{
  free(self);
}
