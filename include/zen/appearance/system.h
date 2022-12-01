#pragma once

#include <wayland-server-core.h>

#include "zen/renderer/session.h"

struct zna_system;

/**
 * @param session is nullable. If not null, the session is owned and destroyed
 * by the zna_system
 */
void zna_system_set_current_session(
    struct zna_system *self, struct znr_session *session);

struct zna_system *zna_system_create(struct wl_display *display);

void zna_system_destroy(struct zna_system *self);
