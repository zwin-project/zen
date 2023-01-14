#include "zwnr/mem-storage.h"

struct zwnr_mem_storage_impl {
  struct zwnr_mem_storage base;

  int ref_count;
};
