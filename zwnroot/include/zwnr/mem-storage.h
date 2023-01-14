#pragma once

#include <sys/types.h>
#ifdef __cplusplus
extern "C" {
#endif

struct zwnr_mem_storage {
  void *data;
  ssize_t size;
};

/**
 * @param src if null, memory is allocated but nothing is copied
 */
struct zwnr_mem_storage *zwnr_mem_storage_create(void *src, size_t size);

struct zwnr_mem_storage *zwnr_mem_storage_ref(struct zwnr_mem_storage *self);

void zwnr_mem_storage_unref(struct zwnr_mem_storage *self);

#ifdef __cplusplus
}
#endif
