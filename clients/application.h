#pragma once

#include <wayland-client.h>
#include <zigen-client-protocol.h>
#include <zigen-gles-v32-client-protocol.h>

#include <memory>

#include "common.h"
#include "loop.h"

namespace zen::client {

class Application
{
 public:
  DISABLE_MOVE_AND_COPY(Application);
  Application() = default;
  ~Application();

  bool Init();

  bool Connect(const char* socket = NULL);

  int Run();

  inline Loop* loop();
  inline zgn_compositor* compositor();
  inline zgn_gles_v32* gles_v32();

 private:
  static const struct wl_registry_listener registry_listener_;
  static void GlobalRegistry(void* data, struct wl_registry* registry,
      uint32_t id, const char* interface, uint32_t version);
  static void GlobalRegistryRemove(
      void* data, struct wl_registry* registry, uint32_t id);

  void Poll();

  wl_display* display_ = nullptr;
  wl_registry* registry_ = nullptr;
  zgn_compositor* zgn_compositor_ = nullptr;
  zgn_gles_v32* zgn_gles_v32_ = nullptr;
  Loop loop_;
  std::unique_ptr<EventSource> event_source_;
};

inline Loop*
Application::loop()
{
  return &loop_;
}

inline zgn_compositor*
Application::compositor()
{
  return zgn_compositor_;
}

inline zgn_gles_v32*
Application::gles_v32()
{
  return zgn_gles_v32_;
}

}  // namespace zen::client