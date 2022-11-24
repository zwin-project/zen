#include "node.h"

#include <zen-common.h>

void
zgnr_region_node_add_cuboid(
    struct zgnr_region_node* self, struct zgnr_cuboid_region* cuboid)
{
  wl_list_insert(&self->cuboid_list, &cuboid->link);
}

struct zgnr_region_node*
zgnr_region_node_create_copy(struct zgnr_region_node* self)
{
  struct zgnr_region_node* copy = zgnr_region_node_create();

  struct zgnr_cuboid_region* cuboid;
  wl_list_for_each (cuboid, &self->cuboid_list, link) {
    struct zgnr_cuboid_region* cuboid_copy = zgnr_cuboid_region_create(
        cuboid->half_size, cuboid->center, cuboid->quaternion);
    zgnr_region_node_add_cuboid(copy, cuboid_copy);
  }

  return copy;
}

struct zgnr_region_node*
zgnr_region_node_create(void)
{
  struct zgnr_region_node* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wl_list_init(&self->cuboid_list);

  return self;

err:
  return NULL;
}

void
zgnr_region_node_destroy(struct zgnr_region_node* self)
{
  struct zgnr_cuboid_region *cuboid, *tmp;

  wl_list_for_each_safe (cuboid, tmp, &self->cuboid_list, link) {
    zgnr_cuboid_region_destroy(cuboid);
  }

  free(self);
}
