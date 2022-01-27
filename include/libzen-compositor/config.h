#ifndef LIBZEN_COMPOSITOR_CONFIG_H
#define LIBZEN_COMPOSITOR_CONFIG_H

enum zen_option_type {
  ZEN_OPTION_INTEGER,
  ZEN_OPTION_STRING,
  ZEN_OPTION_BOOLEAN,
};

struct zen_option {
  enum zen_option_type type;
  const char *name;
  void *data;
};

bool parse_config(const struct zen_option *options, int count, int argc,
    char const *argv[], char const *config_path);

#endif  //  LIBZEN_COMPOSITOR_CONFIG_
