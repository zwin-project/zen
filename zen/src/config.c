#include "zen/config.h"

#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <toml.h>
#include <unistd.h>
#include <wayland-util.h>

#include "zen-common/log.h"
#include "zen-common/util.h"

/// return value must be freed by caller
static char *
get_config_path(void)
{
  const char *config_dir_partial_path = "/.config";
  const char *toml_file_partial_path = "/zen-desktop/config.toml";
  char *config_home_env = NULL;
  char *config_path = NULL;

  config_home_env = getenv("XDG_CONFIG_HOME");

  if (config_home_env == NULL) {
    char *homedir_env = getenv("HOME");

    if (homedir_env == NULL) {
      homedir_env = getpwuid(getuid())->pw_dir;
    }

    if (homedir_env == NULL) {
      return NULL;
    }

    config_path =
        (char *)malloc(strlen(homedir_env) + strlen(config_dir_partial_path) +
                       strlen(toml_file_partial_path) + 1);

    // NOLINTNEXTLINE(cert-err33-c)
    sprintf(config_path, "%s%s%s", homedir_env, config_dir_partial_path,
        toml_file_partial_path);
  } else {
    config_path = (char *)malloc(
        strlen(config_home_env) + strlen(toml_file_partial_path) + 1);

    // NOLINTNEXTLINE(cert-err33-c)
    sprintf(config_path, "%s%s", config_home_env, toml_file_partial_path);
  }

  return config_path;
}

/// return value must be freed by caller with toml_free()
static toml_table_t *
get_config_toml_table(void)
{
  toml_table_t *table = NULL;

  char *config_path = get_config_path();
  if (config_path == NULL) {
    zn_warn("Failed to resolve config path");
    goto out;
  }

  FILE *fp = fopen(config_path, "re");
  if (fp == NULL) {
    zn_warn("Failed to open config file(%s)", config_path);
    goto out_path;
  }

  char err[200];
  table = toml_parse_file(fp, err, sizeof(err));

  if (table == NULL) {
    zn_warn("Failed to parse %s", config_path);
    goto out_fp;
  }

out_fp:
  fclose(fp);  // NOLINT(cert-err33-c)

out_path:
  free(config_path);

out:
  return table;
}

bool
zn_config_add_section(
    struct zn_config *self, const char *name, struct zn_config_section *section)
{
  struct zn_config_section_info *info = NULL;
  wl_array_for_each (info, &self->sections) {
    if (strcmp(info->name, name) == 0) {
      zn_error("Config section %s is used by two or more modules", name);
      return false;
    }
  }

  toml_table_t *table = NULL;
  if (self->root_table) {
    table = toml_table_in(self->root_table, name);
  }

  if (!section->load(section, table)) {
    zn_error("Failed to load '%s' config section", name);
    return false;
  }

  char *name_copy = strdup(name);
  if (name_copy == NULL) {
    zn_error("Failed to strdup");
    return false;
  }

  info = wl_array_add(&self->sections, sizeof(*info));
  if (info == NULL) {
    zn_error("Failed to allocate memory");
    free(name_copy);
    return false;
  }

  info->name = name_copy;
  info->section = section;

  return true;
}

struct zn_config_section *
zn_config_get_section(struct zn_config *self, const char *name)
{
  struct zn_config_section_info *info = NULL;
  wl_array_for_each (info, &self->sections) {
    if (strcmp(info->name, name) == 0) {
      return info->section;
    }
  }

  return NULL;
}

struct zn_config *
zn_config_create(void)
{
  struct zn_config *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->root_table = get_config_toml_table();
  wl_array_init(&self->sections);

  return self;

err:
  return NULL;
}

void
zn_config_destroy(struct zn_config *self)
{
  if (self->root_table) {
    toml_free(self->root_table);
  }

  struct zn_config_section_info *info = NULL;
  wl_array_for_each (info, &self->sections) {
    free(info->name);
    info->section->destroy(info->section);
  }

  wl_array_release(&self->sections);
  free(self);
}
