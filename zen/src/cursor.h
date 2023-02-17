#pragma once

#include "zen/cursor.h"

struct zn_cursor *zn_cursor_create(void);

void zn_cursor_destroy(struct zn_cursor *self);
