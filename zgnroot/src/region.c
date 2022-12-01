#include "region.h"

#include <cglm/types.h>
#include <zen-common.h>
#include <zigen-protocol.h>

#include "region/cuboid.h"
#include "region/sphere.h"

static void zgnr_region_destroy(struct zgnr_region* self);

static void
zgnr_region_handle_destroy(struct wl_resource* resource)
{
  struct zgnr_region* self = wl_resource_get_user_data(resource);
  zgnr_region_destroy(self);
}

static void
zgnr_region_protocol_destroy(
    struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void
zgnr_region_protocol_add_cuboid(struct wl_client* client,
    struct wl_resource* resource, struct wl_array* half_size_array,
    struct wl_array* center_array, struct wl_array* quaternion_array)
{
  struct zgnr_region* self = wl_resource_get_user_data(resource);
  struct zgnr_cuboid_region* cuboid;

  vec3 half_size;
  vec3 center;
  versor quaternion;

  if (zn_array_to_vec3(half_size_array, half_size) != 0) {
    wl_resource_post_error(resource, ZGN_COMPOSITOR_ERROR_WL_ARRAY_SIZE,
        "half_size is expected vec3 (%ld bytes) but got %ld bytes",
        sizeof(vec3), half_size_array->size);
    return;
  }

  if (zn_array_to_vec3(center_array, center) != 0) {
    wl_resource_post_error(resource, ZGN_COMPOSITOR_ERROR_WL_ARRAY_SIZE,
        "center is expected vec3 (%ld bytes) but got %ld bytes", sizeof(vec3),
        center_array->size);
    return;
  }

  if (zn_array_to_versor(quaternion_array, quaternion) != 0) {
    wl_resource_post_error(resource, ZGN_COMPOSITOR_ERROR_WL_ARRAY_SIZE,
        "quaternion is expected vec4 (%ld bytes) but got %ld bytes",
        sizeof(versor), quaternion_array->size);
    return;
  }

  cuboid = zgnr_cuboid_region_create(half_size, center, quaternion);
  if (cuboid == NULL) {
    wl_client_post_no_memory(client);
    zn_error("Failed to creat a cuboid region");
    return;
  }

  zgnr_region_node_add_cuboid(self->node, cuboid);
}

static void
zgnr_region_protocol_add_sphere(struct wl_client* client,
    struct wl_resource* resource, struct wl_array* center_wl_array,
    struct wl_array* radius_wl_array)
{
  struct zgnr_region* self = wl_resource_get_user_data(resource);
  struct zgnr_sphere_region* sphere;

  vec3 center;
  float radius;

  if (zn_array_to_vec3(center_wl_array, center) != 0) {
    wl_resource_post_error(resource, ZGN_COMPOSITOR_ERROR_WL_ARRAY_SIZE,
        "center is expected vec3 (%ld bytes) but got %ld bytes", sizeof(vec3),
        center_wl_array->size);
    return;
  }

  if (zn_array_to_float(radius_wl_array, &radius) != 0) {
    wl_resource_post_error(resource, ZGN_COMPOSITOR_ERROR_WL_ARRAY_SIZE,
        "radius is expected float (%ld bytes) but got %ld bytes", sizeof(float),
        radius_wl_array->size);
    return;
  }

  sphere = zgnr_sphere_region_create(center, radius);
  if (sphere == NULL) {
    wl_client_post_no_memory(client);
    zn_error("Failed to creat a sphere region");
    return;
  }

  zgnr_region_node_add_sphere(self->node, sphere);
}

static const struct zgn_region_interface implementation = {
    .destroy = zgnr_region_protocol_destroy,
    .add_cuboid = zgnr_region_protocol_add_cuboid,
    .add_sphere = zgnr_region_protocol_add_sphere,
};

struct zgnr_region*
zgnr_region_create(struct wl_client* client, uint32_t id)
{
  struct zgnr_region* self;
  struct wl_resource* resource;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->node = zgnr_region_node_create();
  if (self->node == NULL) {
    zn_error("Failed to create zgnr_region_node");
    goto err_free;
  }

  resource = wl_resource_create(client, &zgn_region_interface, 1, id);
  if (resource == NULL) {
    zn_error("Failed to create a wl_resource");
    goto err_node;
  }

  wl_resource_set_implementation(
      resource, &implementation, self, zgnr_region_handle_destroy);

  return self;

err_node:
  zgnr_region_node_destroy(self->node);

err_free:
  free(self);

err:
  return NULL;
}

static void
zgnr_region_destroy(struct zgnr_region* self)
{
  zgnr_region_node_destroy(self->node);

  free(self);
}
