#pragma once

#include "zgnr/space-manager.h"

struct zgnr_space_manager_impl {
  struct zgnr_space_manager base;

  struct wl_global *global;
};
