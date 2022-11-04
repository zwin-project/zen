#pragma once

#include "zen/appearance/scene/virtual-object.h"

struct zna_virtual_object {
  struct zn_virtual_object* virtual_object;  // nonnull
  struct zna_system* system;                 // nonnull
};
