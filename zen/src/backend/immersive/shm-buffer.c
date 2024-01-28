#include "shm-buffer.h"

#include <pthread.h>
#include <signal.h>
#include <sys/mman.h>
#include <zwin-protocol.h>

#include "shm-pool.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/buffer.h"

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static pthread_once_t zn_shm_sigbus_once = PTHREAD_ONCE_INIT;
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static pthread_key_t zn_shm_sigbus_data_key;
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static struct sigaction zn_shm_old_sigbus_action;

struct zn_shm_sigbus_data {
  struct zn_shm_pool *current_pool;
  int access_count;
  int fallback_mapping_used;
};

static void zn_shm_buffer_destroy(struct zn_shm_buffer *self);

static void
reraise_sigbus(void)
{
  sigaction(SIGBUS, &zn_shm_old_sigbus_action, NULL);
  raise(SIGBUS);  // NOLINT(cert-err33-c)
}

static void
sigbus_handler(int signum UNUSED, siginfo_t *info, void *context UNUSED)
{
  struct zn_shm_sigbus_data *sigbus_data =
      pthread_getspecific(zn_shm_sigbus_data_key);
  struct zn_shm_pool *pool = NULL;

  if (sigbus_data == NULL) {
    reraise_sigbus();
    return;
  }

  pool = sigbus_data->current_pool;

  if (pool == NULL || (char *)info->si_addr < pool->data ||
      (char *)info->si_addr >= pool->data + pool->size) {
    reraise_sigbus();
    return;
  }

  sigbus_data->fallback_mapping_used = 1;

  if (mmap(pool->data, pool->size, PROT_READ | PROT_WRITE,
          MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, 0,
          0) == MAP_FAILED) {  // NOLINT(performance-no-int-to-ptr)
    reraise_sigbus();
    return;
  }
}

static void
destroy_sigbus_data(void *data)
{
  struct zn_shm_sigbus_data *sigbus_data = data;

  free(sigbus_data);
}

static void
init_sigbus_data_key(void)
{
  struct sigaction new_action = {
      .sa_sigaction = sigbus_handler,
      .sa_flags = SA_SIGINFO | SA_NODEFER,
  };

  sigemptyset(&new_action.sa_mask);

  sigaction(SIGBUS, &new_action, &zn_shm_old_sigbus_action);

  pthread_key_create(&zn_shm_sigbus_data_key, destroy_sigbus_data);
}

static void
zn_shm_buffer_ref(struct zn_buffer *zn_buffer)
{
  struct zn_shm_buffer *self = (struct zn_shm_buffer *)(zn_buffer->impl_data);

  zn_assert(self->refcount > 0, "invalid refcount status");

  self->refcount++;
}

static void
zn_shm_buffer_unref(struct zn_buffer *zn_buffer)
{
  struct zn_shm_buffer *self = (struct zn_shm_buffer *)(zn_buffer->impl_data);

  self->refcount--;
  zn_assert(self->refcount >= 0, "invalid refcount status");

  if (self->refcount == 0) {
    zn_shm_buffer_destroy(self);
  }
}

static void *
zn_shm_buffer_begin_access(struct zn_buffer *zn_buffer)
{
  struct zn_shm_buffer *self = (struct zn_shm_buffer *)(zn_buffer->impl_data);
  struct zn_shm_pool *pool = self->pool;

  if (self->pool->external_refcount &&
      self->pool->size != self->pool->new_size) {
    zn_warn(
        "Buffer address requested when its parent pool has an external "
        "reference and a deferred resize pending.");
  }

  void *data = self->pool->data + self->offset;

  if (pool->sigbuf_is_impossible) {
    return data;
  }

  pthread_once(&zn_shm_sigbus_once, init_sigbus_data_key);

  struct zn_shm_sigbus_data *sigbus_data =
      pthread_getspecific(zn_shm_sigbus_data_key);
  if (sigbus_data == NULL) {
    sigbus_data = zalloc(sizeof *sigbus_data);
    if (sigbus_data == NULL) {
      return NULL;
    }

    pthread_setspecific(zn_shm_sigbus_data_key, sigbus_data);
  }

  zn_assert(
      sigbus_data->current_pool == NULL || sigbus_data->current_pool == pool,
      "Accessing a pool data while accessing another pool data is prohibited");

  sigbus_data->current_pool = pool;
  sigbus_data->access_count++;

  return data;
}

static bool
zn_shm_buffer_end_access(struct zn_buffer *zn_buffer)
{
  struct zn_shm_buffer *self = (struct zn_shm_buffer *)(zn_buffer->impl_data);
  struct zn_shm_pool *pool = self->pool;

  if (pool->sigbuf_is_impossible) {
    return true;
  }

  struct zn_shm_sigbus_data *sigbus_data =
      pthread_getspecific(zn_shm_sigbus_data_key);
  if (!zn_assert(sigbus_data && sigbus_data->access_count >= 1,
          "More zn_shm_buffer_end_access was called than the previously called "
          "zn_shm_buffer_begin_access.")) {
    return false;
  }

  bool status = true;

  if (--sigbus_data->access_count == 0) {
    if (sigbus_data->fallback_mapping_used) {
      status = false;
      if (self->resource != NULL) {
        wl_resource_post_error(self->resource, ZWN_SHM_ERROR_INVALID_FD,
            "error accessing SHM buffer");
      }
      sigbus_data->fallback_mapping_used = 0;
    }

    sigbus_data->current_pool = NULL;
  }

  return status;
}

static ssize_t
zn_shm_buffer_get_size(struct zn_buffer *zn_buffer)
{
  struct zn_shm_buffer *self = (struct zn_shm_buffer *)(zn_buffer->impl_data);

  return self->size;
}

static const struct zn_buffer_interface buffer_implementation = {
    .ref = zn_shm_buffer_ref,
    .unref = zn_shm_buffer_unref,
    .begin_access = zn_shm_buffer_begin_access,
    .end_access = zn_shm_buffer_end_access,
    .get_size = zn_shm_buffer_get_size,
};

static void
zn_shm_buffer_handle_destroy(struct wl_resource *resource)
{
  struct zn_shm_buffer *self = wl_resource_get_user_data(resource);

  self->resource = NULL;

  zn_shm_buffer_unref(self->zn_buffer);
}

static void
zn_shm_buffer_protocol_destroy(
    struct wl_client *client UNUSED, struct wl_resource *resource)
{
  wl_resource_destroy(resource);
}

static const struct zwn_buffer_interface implementation = {
    .destroy = zn_shm_buffer_protocol_destroy,
};

struct zn_shm_buffer *
zn_shm_buffer_get(struct wl_resource *resource)
{
  if (resource == NULL) {
    return NULL;
  }

  if (!zn_assert(wl_resource_instance_of(
                     resource, &zwn_buffer_interface, &implementation),
          "resource is not zn_shm_buffer")) {
    return NULL;
  }

  return wl_resource_get_user_data(resource);
}

struct zn_shm_buffer *
zn_shm_buffer_create(struct wl_client *client, uint32_t id, int64_t offset,
    int64_t size, struct wl_resource *pool_resource)
{
  struct zn_shm_pool *pool = wl_resource_get_user_data(pool_resource);

  if (offset < 0 || size <= 0 || offset > pool->size - size) {
    wl_resource_post_error(
        pool_resource, ZWN_SHM_ERROR_INVALID_SIZE, "invalid size (%ld)", size);
    goto err;
  }

  struct zn_shm_buffer *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    wl_client_post_no_memory(client);
    goto err;
  }

  self->refcount = 1;
  self->size = size;
  self->offset = offset;
  self->pool = pool;

  self->zn_buffer = zn_buffer_create(self, &buffer_implementation);
  if (self->zn_buffer == NULL) {
    zn_error("Failed to create zn_buffer");
    goto err_free;
  }

  pool->internal_refcount++;

  self->resource = wl_resource_create(client, &zwn_buffer_interface, 1, id);
  if (self->resource == NULL) {
    wl_client_post_no_memory(client);
    goto err_ref;
  }

  wl_resource_set_implementation(
      self->resource, &implementation, self, zn_shm_buffer_handle_destroy);

  return self;

err_ref:
  zn_shm_pool_unref(pool, false);
  zn_buffer_destroy(self->zn_buffer);

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_shm_buffer_destroy(struct zn_shm_buffer *self)
{
  zn_shm_pool_unref(self->pool, false);
  zn_buffer_destroy(self->zn_buffer);
  free(self);
}
