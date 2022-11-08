#include "shm-pool.h"

#include <zen-common.h>

#include "application.h"

namespace zen::client {

bool
ShmPool::Init(int fd, off_t size)
{
  wl_array size_array;
  zn_off_t_to_array(size, &size_array);

  proxy_ = zgn_shm_create_pool(app_->shm(), fd, &size_array);
  if (proxy_ == nullptr) {
    zn_error("Failed to create shm pool proxy");
    return false;
  }

  wl_array_release(&size_array);

  return true;
}

ShmPool::ShmPool(Application* app) : app_(app) {}

ShmPool::~ShmPool()
{
  if (proxy_) {
    zgn_shm_pool_destroy(proxy_);
  }
}

std::unique_ptr<ShmPool>
CreateShmPool(Application* app, int fd, off_t size)
{
  auto pool = std::make_unique<ShmPool>(app);

  if (!pool->Init(fd, size)) {
    return std::unique_ptr<ShmPool>();
  }

  return pool;
}

}  // namespace zen::client
