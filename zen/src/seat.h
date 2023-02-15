#pragma once

#include "zen/seat.h"

struct zn_seat *zn_seat_create(void);

void zn_seat_destroy(struct zn_seat *self);
