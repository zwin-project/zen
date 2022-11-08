#include "buffer.h"

#include <zen-common.h>

#include "shm-pool.h"

namespace zen::client {

bool
Buffer::Init(ShmPool* pool, off_t offset, off_t size)
{
  wl_array offset_array, size_array;
  zn_off_t_to_array(offset, &offset_array);
  zn_off_t_to_array(size, &size_array);

  proxy_ =
      zgn_shm_pool_create_buffer(pool->proxy(), &offset_array, &size_array);
  if (proxy_ == nullptr) {
    zn_error("Failed to create zgn buffer proxy");
    return false;
  }

  wl_array_release(&offset_array);
  wl_array_release(&size_array);

  return true;
}

Buffer::~Buffer()
{
  if (proxy_) {
    zgn_buffer_destroy(proxy_);
  }
}

std::unique_ptr<Buffer>
CreateBuffer(ShmPool* pool, off_t offset, off_t size)
{
  auto buffer = std::make_unique<Buffer>();

  if (!buffer->Init(pool, offset, size)) {
    return std::unique_ptr<Buffer>();
  }

  return buffer;
}

}  // namespace zen::client
