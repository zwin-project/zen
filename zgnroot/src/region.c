#include "region.h"

#include <zen-common.h>
#include <zigen-protocol.h>

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
  UNUSED(client);

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

  cuboid = zalloc(sizeof *cuboid);
  glm_vec3_copy(half_size, cuboid->half_size);
  glm_vec3_copy(center, cuboid->center);
  glm_vec4_copy(quaternion, cuboid->quaternion);

  wl_list_insert(&self->cuboid_list, &cuboid->link);
}

static const struct zgn_region_interface implementation = {
    .destroy = zgnr_region_protocol_destroy,
    .add_cuboid = zgnr_region_protocol_add_cuboid,
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

  resource = wl_resource_create(client, &zgn_region_interface, 1, id);
  if (resource == NULL) {
    zn_error("Failed to create a wl_resource");
    goto err_free;
  }

  wl_list_init(&self->cuboid_list);

  wl_resource_set_implementation(
      resource, &implementation, self, zgnr_region_handle_destroy);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zgnr_region_destroy(struct zgnr_region* self)
{
  struct zgnr_cuboid_region *cuboid, *tmp;

  wl_list_for_each_safe (cuboid, tmp, &self->cuboid_list, link) {
    wl_list_remove(&cuboid->link);
    free(cuboid);
  }

  wl_list_remove(&self->cuboid_list);
  free(self);
}
