#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "znr/remote.h"

struct znr_virtual_object {
  uint64_t id;
};

void znr_virtual_object_commit(struct znr_virtual_object* self);

struct znr_virtual_object* znr_virtual_object_create(struct znr_remote* remote);

void znr_virtual_object_destroy(struct znr_virtual_object* self);

#ifdef __cplusplus
}
#endif
