#include <assert.h>

#include "config.c"  // NOLINT(bugprone-suspicious-include)
#include "test-harness.h"

TEST(get_config_path_from_xdg_config_home)
{
  setenv("XDG_CONFIG_HOME", "/path/to/config", 1);
  char *config_path = get_config_path();

  ASSERT_EQUAL_STRING("/path/to/config/zen-desktop/config.toml", config_path);
}

TEST(get_config_path_from_home)
{
  unsetenv("XDG_CONFIG_HOME");
  setenv("HOME", "/path/to/home", 1);
  char *config_path = get_config_path();

  ASSERT_EQUAL_STRING(
      "/path/to/home/.config/zen-desktop/config.toml", config_path);
}
