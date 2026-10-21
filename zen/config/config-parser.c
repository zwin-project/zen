#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tomlc17.h>
#include <unistd.h>
#include <zen-common.h>

static char *
get_config_filepath(void)
{
  const char *config_dir_partial_path = "/.config";
  const char *toml_file_partial_path = "/zen-desktop/config.toml";
  char *config_home_env, *config_home;
  config_home_env = getenv("XDG_CONFIG_HOME");
  if (config_home_env == NULL) {
    char *homedir_env = getenv("HOME");
    if (homedir_env == NULL) homedir_env = getpwuid(getuid())->pw_dir;
    if (homedir_env == NULL) {
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
  return config_home;
}

toml_result_t
zn_config_get_toml_result(void)
{
  toml_result_t result = {0};

  char *config_filepath = get_config_filepath();
  if (config_filepath == NULL) {
    zn_warn("Could not find the home directory");
    return result;
  }

  FILE *fp = fopen(config_filepath, "r");
  if (fp == NULL) {
    zn_warn("Failed to open %s\n", config_filepath);
    free(config_filepath);
    return result;
  }
  free(config_filepath);

  result = toml_parse_file(fp);
  fclose(fp);

  if (!result.ok) zn_warn("Failed to parse config.toml: %s\n", result.errmsg);
  return result;
}
