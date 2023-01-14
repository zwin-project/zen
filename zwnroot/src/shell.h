#pragma once

#include "zwnr/shell.h"

struct zwnr_shell_impl {
  struct zwnr_shell base;

  struct wl_global *global;
};
