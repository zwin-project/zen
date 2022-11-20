#include "mem-storage.h"

#include <zen-common.h>

static void zgnr_mem_storage_destroy(struct zgnr_mem_storage_impl* self);

struct zgnr_mem_storage*
zgnr_mem_storage_ref(struct zgnr_mem_storage* parent)
{
  struct zgnr_mem_storage_impl* self = zn_container_of(parent, self, base);
  self->ref_count++;
  return &self->base;
}

void
zgnr_mem_storage_unref(struct zgnr_mem_storage* parent)
{
  struct zgnr_mem_storage_impl* self = zn_container_of(parent, self, base);
  if (--self->ref_count == 0) {
    zgnr_mem_storage_destroy(self);
  }
}

struct zgnr_mem_storage*
zgnr_mem_storage_create(void* src, size_t size)
{
  struct zgnr_mem_storage_impl* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    goto err;
  }

  self->base.size = size;
  self->ref_count = 1;

  self->base.data = malloc(size);
  if (self->base.data == NULL) {
    zn_error("Failed to allocate memory");
    goto err_free;
  }

  if (src) memcpy(self->base.data, src, size);

  return &self->base;

err_free:
  free(self);

err:
  return NULL;
}

static void
zgnr_mem_storage_destroy(struct zgnr_mem_storage_impl* self)
{
  free(self->base.data);
  free(self);
}
