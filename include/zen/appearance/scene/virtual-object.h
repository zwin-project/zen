#pragma once

#include "zen/appearance/appearance.h"
#include "zen/scene/virtual-object.h"

struct zn_virtual_object_appearance;

struct zn_virtual_object_appearance* zn_virtual_object_appearance_create(
    struct zn_virtual_object* virtual_object, struct zn_appearance* appearance);

void zn_virtual_object_appearance_destroy(
    struct zn_virtual_object_appearance* self);
