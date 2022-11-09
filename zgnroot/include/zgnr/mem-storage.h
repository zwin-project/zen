#pragma once

#include <sys/types.h>

struct zgnr_mem_storage {
  void* data;
  ssize_t size;
};

struct zgnr_mem_storage* zgnr_mem_storage_ref(struct zgnr_mem_storage* self);

void zgnr_mem_storage_unref(struct zgnr_mem_storage* self);
