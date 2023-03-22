#include "xr-system-private.h"

namespace zen::xr::proxy {

XrSystemProxy::XrSystemProxy(uint64_t handle) { zn_xr_system_.handle = handle; }

bool
XrSystemProxy::Init()
{
  zn_xr_system_.impl_data = this;

  wl_signal_init(&zn_xr_system_.events.destroy);

  return true;
}

XrSystemProxy::~XrSystemProxy()
{
  wl_signal_emit(&zn_xr_system_.events.destroy, nullptr);
  wl_list_remove(&zn_xr_system_.events.destroy.listener_list);
}

}  // namespace zen::xr::proxy
