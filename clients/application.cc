#include "application.h"

#include <errno.h>

#include <cstring>

namespace zen::client {

void
Application::GlobalRegistry(void* data, struct wl_registry* registry,
    uint32_t id, const char* interface, uint32_t version)
{
  auto app = static_cast<Application*>(data);
  if (std::strcmp(interface, "zgn_compositor") == 0) {
    app->zgn_compositor_ = static_cast<zgn_compositor*>(
        wl_registry_bind(registry, id, &zgn_compositor_interface, version));
  } else if (std::strcmp(interface, "zgn_gles_v32") == 0) {
    app->zgn_gles_v32_ = static_cast<zgn_gles_v32*>(
        wl_registry_bind(registry, id, &zgn_gles_v32_interface, version));
  } else if (std::strcmp(interface, "zgn_shm") == 0) {
    app->zgn_shm_ = static_cast<zgn_shm*>(
        wl_registry_bind(registry, id, &zgn_shm_interface, version));
  }
}

void
Application::GlobalRegistryRemove(
    void* /*data*/, struct wl_registry* /*registry*/, uint32_t /*id*/)
{}

bool
Application::Init()
{
  if (loop_.Init() == false) {
    zn_error("Failed to initialize loop");
    return false;
  }
  return true;
}

bool
Application::Connect(const char* socket)
{
  if (socket == NULL) socket = std::getenv("WAYLAND_DISPLAY");
  if (socket == NULL) socket = "wayland-0";

  display_ = wl_display_connect(socket);
  if (display_ == nullptr) {
    zn_error("Failed to connect to wayland server: %s", socket);
    goto err;
  }

  registry_ = wl_display_get_registry(display_);
  if (registry_ == nullptr) {
    zn_error("Failed to connect to wayland server: %s", socket);
    goto err_disconnect;
  }

  wl_registry_add_listener(registry_, &Application::registry_listener_, this);

  wl_display_roundtrip(display_);

  if (zgn_compositor_ == nullptr || zgn_gles_v32_ == nullptr ||
      zgn_shm_ == nullptr) {
    zn_error("Server does not support zigen protocols");
    goto err_globals;
  }

  event_source_ = loop_.AddFd(
      wl_display_get_fd(display_), Loop::kEventReadable,
      [](int /*fd*/, uint32_t mask, void* data) {
        auto app = static_cast<Application*>(data);
        if (mask & Loop::kEventError || mask & Loop::kEventHangUp) {
          zn_error("Disconnected from wayland server");
          app->loop()->Terminate(EXIT_FAILURE);
        } else if (mask == Loop::kEventReadable) {
          app->Poll();
        }
      },
      this);

  return true;

err_globals:
  if (zgn_gles_v32_) zgn_gles_v32_destroy(zgn_gles_v32_);
  zgn_gles_v32_ = nullptr;

  if (zgn_compositor_) zgn_compositor_destroy(zgn_compositor_);
  zgn_compositor_ = nullptr;

  wl_registry_destroy(registry_);
  registry_ = nullptr;

err_disconnect:
  wl_display_disconnect(display_);
  display_ = nullptr;

err:
  return false;
}

void
Application::Poll()
{
  while (wl_display_prepare_read(display_) != 0) {
    if (errno != EAGAIN) {
      loop_.Terminate(EXIT_FAILURE);
      return;
    }
    wl_display_dispatch_pending(display_);
  }

  if (wl_display_flush(display_) == -1) {
    loop_.Terminate(EXIT_FAILURE);
    return;
  }
  wl_display_read_events(display_);
  wl_display_dispatch_pending(display_);

  if (wl_display_flush(display_) == -1) {
    loop_.Terminate(EXIT_FAILURE);
    return;
  }
}

int
Application::Run()
{
  wl_display_flush(display_);
  return loop_.Run();
}

Application::~Application()
{
  if (zgn_shm_) zgn_shm_destroy(zgn_shm_);
  if (zgn_gles_v32_) zgn_gles_v32_destroy(zgn_gles_v32_);
  if (zgn_compositor_) zgn_compositor_destroy(zgn_compositor_);
  if (registry_) wl_registry_destroy(registry_);
  if (display_) wl_display_disconnect(display_);
}

const struct wl_registry_listener Application::registry_listener_ = {
    Application::GlobalRegistry,
    Application::GlobalRegistryRemove,
};

}  // namespace zen::client
