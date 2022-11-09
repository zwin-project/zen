#include "zgnr/mem-storage.h"

struct zgnr_mem_storage_impl {
  struct zgnr_mem_storage base;

  int ref_count;
};

struct zgnr_mem_storage* zgnr_mem_storage_create(void* src, size_t size);
