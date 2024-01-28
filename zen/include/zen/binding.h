#pragma once

#include <wlr/types/wlr_keyboard.h>

#include "zen/config.h"

struct zn_binding;

/// @return true if the key is used and should not be used any more
typedef bool (*zn_binding_handler_t)(
    const char *name, uint32_t key, uint32_t modifiers, void *user_data);

/// @param name must be a string literal
void zn_binding_add(struct zn_binding *self, const char *name,
    zn_binding_handler_t handler, void *user_data);
