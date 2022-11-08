#pragma once

#include <zen-common.h>
#include <zigen-client-protocol.h>

#include <memory>

namespace zen::client {

class ShmPool;

class Buffer
{
 public:
  DISABLE_MOVE_AND_COPY(Buffer);
  Buffer() = default;
  ~Buffer();

  bool Init(ShmPool* pool, off_t offset, off_t size);

  inline zgn_buffer* proxy();

 private:
  zgn_buffer* proxy_ = nullptr;  // nonnull after initialization
};

inline zgn_buffer*
Buffer::proxy()
{
  return proxy_;
}

std::unique_ptr<Buffer> CreateBuffer(ShmPool* pool, off_t offset, off_t size);

}  // namespace zen::client
