#pragma once

#include "zen/cursor.h"

struct zn_cursor_impl {
  struct zn_cursor base;
};

void zn_cursor_impl_notify_motion(
    struct zn_cursor_impl *self, struct zn_cursor_motion_event *event);

struct zn_cursor_impl *zn_cursor_impl_get(struct zn_cursor *base);

struct zn_cursor_impl *zn_cursor_impl_create(void);

void zn_cursor_impl_destroy(struct zn_cursor_impl *self);
