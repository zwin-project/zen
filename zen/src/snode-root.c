#include "zen/snode-root.h"

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/snode.h"

struct zn_snode_root *
zn_snode_root_create(
    void *user_data, const struct zn_snode_root_interface *implementation)
{
  struct zn_snode_root *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->impl = implementation;
  self->user_data = user_data;

  self->node = zn_snode_create(self, &zn_snode_noop_implementation);
  if (self->node == NULL) {
    zn_error("Failed to create root snode");
    goto err_free;
  }

  self->node->root = self;

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_snode_root_destroy(struct zn_snode_root *self)
{
  zn_snode_destroy(self->node);
  free(self);
}
