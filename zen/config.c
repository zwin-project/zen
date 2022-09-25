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
  const char *config_dir_partial_path = "/.config";
  const char *toml_file_partial_path = "/zen-desktop/config.toml";
  char *config_home_env, *config_home;
  config_home_env = getenv("XDG_CONFIG_HOME");
  if (config_home_env == NULL) {
    char *homedir_env = getenv("HOME");
    if (homedir_env == NULL) homedir_env = getpwuid(getuid())->pw_dir;
    if (homedir_env == NULL) {
      zn_warn("Could not find the home directory");
      return NULL;
    }

    config_home =
        (char *)malloc(strlen(homedir_env) + strlen(config_dir_partial_path) +
                       strlen(toml_file_partial_path) + 1);
    sprintf(config_home, "%s%s%s", homedir_env, config_dir_partial_path,
        toml_file_partial_path);
  } else {
    config_home = (char *)malloc(
        strlen(config_home_env) + strlen(toml_file_partial_path) + 1);
    sprintf(config_home, "%s%s", config_home_env, toml_file_partial_path);
  }

  FILE *fp = fopen(config_home, "r");
  free(config_home);
  if (fp == NULL) {
    zn_warn("Failed to open %s\n", config_home);
    return NULL;
  }

  char errbuf[200];
  toml_table_t *tbl = toml_parse_file(fp, errbuf, sizeof(errbuf));
  fclose(fp);

  if (tbl == NULL) zn_warn("Failed to parse config.toml: %s\n", errbuf);
  return tbl;
}

static void
zn_config_set_default(struct zn_config *self)
{
  // It is freed in zen_config_destroy so DEFAULT_WALLPAPER
  // should not be passed directly.
  self->bg_image_file = strdup(DEFAULT_WALLPAPER);
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
    zn_warn("Failed to get the toml table");
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
  free(self->bg_image_file);
  free(self);
}
