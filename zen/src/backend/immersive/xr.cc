#include "xr-private.h"
#include "xr-system-private.h"
#include "zen-common/log.h"

namespace zen::xr::proxy {

XrProxy::XrProxy() = default;

bool
XrProxy::Init(wl_display *display)
{
  zn_xr_.impl_data = this;
  wl_signal_init(&zn_xr_.events.new_system);
  xr_ = zen::xr::CreateXr(display, this);
  return true;
}

XrProxy::~XrProxy() { wl_list_remove(&zn_xr_.events.new_system.listener_list); }

void
XrProxy::NotifySystemAdded(uint64_t handle)
{
  auto system = std::make_unique<XrSystemProxy>(handle);
  if (!system->Init()) {
    zn_warn("Failed to init XrSystemProxy");
    return;
  }
  auto *zn_xr_system = system->zn_xr_system();
  systems_.push_back(std::move(system));

  wl_signal_emit(&zn_xr_.events.new_system, zn_xr_system);
}

void
XrProxy::NotifySystemRemoved(std::unique_ptr<zen::xr::System> &system)
{
  auto result = std::remove_if(systems_.begin(), systems_.end(),
      [handle = system->handle()](
          std::unique_ptr<XrSystemProxy> &system_proxy) {
        return system_proxy->handle() == handle;
      });

  systems_.erase(result, systems_.end());
}

}  // namespace zen::xr::proxy

zn_xr *
zn_xr_create(struct wl_display *display)
{
  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
  auto *proxy = new zen::xr::proxy::XrProxy;

  if (!proxy->Init(display)) {
    zn_error("Failed to initialize XrProxy");
    delete proxy;  // NOLINT(cppcoreguidelines-owning-memory)
    return nullptr;
  }

  return &proxy->zn_xr_;
}

void
zn_xr_destroy(zn_xr *self)
{
  auto *proxy = static_cast<zen::xr::proxy::XrProxy *>(self->impl_data);

  delete proxy;  // NOLINT(cppcoreguidelines-owning-memory)
}
