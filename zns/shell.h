#pragma once

#include <zgnr/shell.h>

#include "zen/shell/shell.h"

struct zn_shell {
  struct zgnr_shell* zgnr_shell;

  struct wl_listener new_bounded_listener;
};
