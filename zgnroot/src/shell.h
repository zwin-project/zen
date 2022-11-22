#pragma once

#include "zgnr/shell.h"

struct zgnr_shell_impl {
  struct zgnr_shell base;

  struct wl_global *global;
};
