#include "zgnr/mem-storage.h"

struct zgnr_mem_storage_impl {
  struct zgnr_mem_storage base;

  int ref_count;
};
