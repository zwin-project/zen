#include "region.h"

#include <cglm/types.h>
#include <zen-common.h>
#include <zwin-protocol.h>

#include "region/cuboid.h"

static void zwnr_region_destroy(struct zwnr_region *self);

static void
zwnr_region_handle_destroy(struct wl_resource *resource)
{
  struct zwnr_region *self = wl_resource_get_user_data(resource);
  zwnr_region_destroy(self);
}

static void
zwnr_region_protocol_destroy(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void
zwnr_region_protocol_add_cuboid(struct wl_client *client,
    struct wl_resource *resource, struct wl_array *half_size_array,
    struct wl_array *center_array, struct wl_array *quaternion_array)
{
  UNUSED(client);

  struct zwnr_region *self = wl_resource_get_user_data(resource);
  struct zwnr_cuboid_region *cuboid;

  vec3 half_size;
  vec3 center;
  versor quaternion;

  if (zn_array_to_vec3(half_size_array, half_size) != 0) {
    wl_resource_post_error(resource, ZWN_COMPOSITOR_ERROR_WL_ARRAY_SIZE,
        "half_size is expected vec3 (%ld bytes) but got %ld bytes",
        sizeof(vec3), half_size_array->size);
    return;
  }

  if (zn_array_to_vec3(center_array, center) != 0) {
    wl_resource_post_error(resource, ZWN_COMPOSITOR_ERROR_WL_ARRAY_SIZE,
        "center is expected vec3 (%ld bytes) but got %ld bytes", sizeof(vec3),
        center_array->size);
    return;
  }

  if (zn_array_to_versor(quaternion_array, quaternion) != 0) {
    wl_resource_post_error(resource, ZWN_COMPOSITOR_ERROR_WL_ARRAY_SIZE,
        "quaternion is expected vec4 (%ld bytes) but got %ld bytes",
        sizeof(versor), quaternion_array->size);
    return;
  }

  cuboid = zwnr_cuboid_region_create(half_size, center, quaternion);
  if (cuboid == NULL) {
    zn_error("Failed to creat a cuboid region");
    return;
  }

  zwnr_region_node_add_cuboid(self->node, cuboid);
}

static const struct zwn_region_interface implementation = {
    .destroy = zwnr_region_protocol_destroy,
    .add_cuboid = zwnr_region_protocol_add_cuboid,
};

struct zwnr_region *
zwnr_region_create(struct wl_client *client, uint32_t id)
{
  struct zwnr_region *self;
  struct wl_resource *resource;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->node = zwnr_region_node_create();
  if (self->node == NULL) {
    zn_error("Failed to create zwnr_region_node");
    goto err_free;
  }

  resource = wl_resource_create(client, &zwn_region_interface, 1, id);
  if (resource == NULL) {
    zn_error("Failed to create a wl_resource");
    goto err_node;
  }

  wl_resource_set_implementation(
      resource, &implementation, self, zwnr_region_handle_destroy);

  return self;

err_node:
  zwnr_region_node_destroy(self->node);

err_free:
  free(self);

err:
  return NULL;
}

static void
zwnr_region_destroy(struct zwnr_region *self)
{
  zwnr_region_node_destroy(self->node);

  free(self);
}
