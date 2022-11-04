#pragma once

#include "zen/appearance/scene/virtual-object.h"

struct zn_virtual_object_appearance {
  struct zn_virtual_object* virtual_object;  // nonnull
  struct zn_appearance* appearance;          // nonnull
};
