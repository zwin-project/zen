#include <ctype.h>
#include <errno.h>
#include <libzen-compositor/libzen-compositor.h>
#include <stdlib.h>
#include <string.h>

const char env_prefix[] = "ZEN_";

static bool
safe_strtoint(const char *str, int32_t *value)
{
  long ret;
  char *end;

  errno = 0;
  ret = strtol(str, &end, 10);
  if (errno != 0) {
    return false;
  } else if (end == str || *end != '\0') {
    errno = EINVAL;
    return false;
  }

  if ((long)((int32_t)ret) != ret) {
    errno = ERANGE;
    return false;
  }

  *value = (int32_t)ret;

  return true;
}

static bool
handle_value(const struct zen_option *option, char *value)
{
  switch (option->type) {
    case ZEN_OPTION_INTEGER:
      if (!safe_strtoint(value, option->data)) {
        zen_log("configuration error: invalid value \"%s\" for %s\n", value,
            option->name);
        return false;
      }
      return true;

    case ZEN_OPTION_STRING:
      *(char **)(option->data) = strdup(value);
      return true;

    case ZEN_OPTION_BOOLEAN:
      *(bool *)option->data =
          !(strcmp(value, "0") == 0 || strcmp(value, "no") == 0 ||
              strcmp(value, "No") == 0 || strcmp(value, "false") == 0 ||
              strcmp(value, "False") == 0);
      return true;
  }
  assert(false && "not reached");
}

static char *
create_env_name(const char *name)
{
  int prefix_len = ARRAY_LENGTH(env_prefix) - 1;
  int len = strlen(name);
  char *env_name = malloc(prefix_len + len + 1);
  strcpy(env_name, env_prefix);
  for (int i = 0; i < len; i++) {
    switch (name[i]) {
      case ' ':
        env_name[prefix_len + i] = '_';
        break;

      default:
        env_name[prefix_len + i] = toupper(name[i]);
    }
  }
  env_name[prefix_len + len] = '\0';
  return env_name;
}

static bool
parse_config_from_env_var(const struct zen_option *options, int count)
{
  char *env_name;
  char *env_value;

  for (const struct zen_option *option = options; option < options + count;
       option++) {
    env_name = create_env_name(option->name);
    env_value = getenv(env_name);
    free(env_name);

    if (env_value && !handle_value(option, env_value)) return false;
  }
  return true;
}

WL_EXPORT bool
parse_config(const struct zen_option *options, int count, int argc,
    char const *argv[], char const *config_path)
{
  // TODO: read config from command line arguments and config file
  UNUSED(argv);
  UNUSED(argc);
  UNUSED(config_path);
  return parse_config_from_env_var(options, count);
}
