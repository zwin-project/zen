#pragma once

#include <zen-common.h>
#include <zigen-client-protocol.h>

#include <memory>

namespace zen::client {

class Application;

class ShmPool
{
 public:
  DISABLE_MOVE_AND_COPY(ShmPool);
  ShmPool(Application* app);
  ~ShmPool();

  bool Init(int fd, off_t size);

  inline zgn_shm_pool* proxy();

 private:
  Application* app_;
  zgn_shm_pool* proxy_ = nullptr;  // nonnull after initialization
};

inline zgn_shm_pool*
ShmPool::proxy()
{
  return proxy_;
}

std::unique_ptr<ShmPool> CreateShmPool(Application* app, int fd, off_t size);

}  // namespace zen::client
