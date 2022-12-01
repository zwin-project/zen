#pragma once

#include "zen/appearance/system.h"
#include "zen/virtual-object.h"

struct zna_virtual_object;

struct zna_virtual_object* zna_virtual_object_create(
    struct zn_virtual_object* virtual_object, struct zna_system* system);

void zna_virtual_object_destroy(struct zna_virtual_object* self);
