#pragma once

#include <sys/types.h>
#ifdef __cplusplus
extern "C" {
#endif

struct zgnr_mem_storage {
  void* data;
  ssize_t size;
};

/**
 * @param src if null, memory is allocated but nothing is copied
 */
struct zgnr_mem_storage* zgnr_mem_storage_create(void* src, size_t size);

struct zgnr_mem_storage* zgnr_mem_storage_ref(struct zgnr_mem_storage* self);

void zgnr_mem_storage_unref(struct zgnr_mem_storage* self);

#ifdef __cplusplus
}
#endif
