#include "remote-system.h"

namespace zen::xr {

RemoteSystem::RemoteSystem(uint64_t handle, uint64_t peer_id)
    : handle_(handle), peer_id_(peer_id)
{}

uint64_t
RemoteSystem::handle()
{
  return handle_;
}

System::Type
RemoteSystem::type()
{
  return kRemote;
}

}  // namespace zen::xr
