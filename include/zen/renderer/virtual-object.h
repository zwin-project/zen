#pragma once

#include "zen/renderer/session.h"

#ifdef __cplusplus
extern "C" {
#endif

struct znr_virtual_object;

void znr_virtual_object_commit(struct znr_virtual_object* self);

struct znr_virtual_object* znr_virtual_object_create(
    struct znr_session* session);

void znr_virtual_object_destroy(struct znr_virtual_object* self);

#ifdef __cplusplus
}
#endif
