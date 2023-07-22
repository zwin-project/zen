#pragma once

#include "zen/view.h"

/// Called by the impl object
void zn_view_notify_resized(struct zn_view *self, vec2 size);

/// Called by the impl object
void zn_view_notify_decoration(
    struct zn_view *self, enum zn_view_decoration_mode mode);

/// Called by the impl object
void zn_view_notify_move_request(struct zn_view *self);

/// Called by the impl object
void zn_view_notify_resize_request(
    struct zn_view *self, struct zn_view_resize_event *event);

/// Called by the impl object
void zn_view_notify_unmap(struct zn_view *self);

/// Called by the impl object
struct zn_view *zn_view_create(
    void *impl_data, const struct zn_view_interface *implementation);

/// Called by the impl object
void zn_view_destroy(struct zn_view *self);
