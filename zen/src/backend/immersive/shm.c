#include "shm.h"

#include <unistd.h>
#include <zwin-protocol.h>

#include "shm-pool.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen-common/wl-array.h"

static void
zn_shm_protocol_create_pool(struct wl_client *client,
    struct wl_resource *resource, uint32_t id, int fd,
    struct wl_array *size_array)
{
  int64_t size = 0;

  if (!zn_wl_array_to_int64_t(size_array, &size)) {
    wl_resource_post_error(resource, ZWN_COMPOSITOR_ERROR_INVALID_WL_ARRAY_SIZE,
        "invalid wl_array size");
    return;
  }

  zn_shm_pool_create(client, id, fd, size, resource);

  close(fd);
}

static const struct zwn_shm_interface implementation = {
    .create_pool = zn_shm_protocol_create_pool,
};

static void
zn_shm_bind(
    struct wl_client *client, void *data UNUSED, uint32_t version, uint32_t id)
{
  struct wl_resource *resource =
      wl_resource_create(client, &zwn_shm_interface, (int)version, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    zn_error("Failed to create a wl_resource");
    return;
  }

  wl_resource_set_implementation(resource, &implementation, NULL, NULL);
}

bool
zn_shm_init(struct wl_display *display)
{
  if (!wl_global_create(display, &zwn_shm_interface, 1, NULL, zn_shm_bind)) {
    return false;
  }

  return true;
}
