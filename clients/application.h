#pragma once

#include <wayland-client.h>
#include <zigen-client-protocol.h>
#include <zigen-gles-v32-client-protocol.h>
#include <zigen-shell-client-protocol.h>

#include <glm/vec3.hpp>
#include <memory>

#include "loop.h"

namespace zen::client {

class VirtualObject;

class Application
{
 public:
  DISABLE_MOVE_AND_COPY(Application);
  Application() = default;
  virtual ~Application();

  bool Init();

  bool Connect(const char* socket = NULL);

  int Run();

  inline Loop* loop();
  inline zgn_compositor* compositor();
  inline zgn_seat* seat();
  inline zgn_gles_v32* gles_v32();
  inline zgn_shell* shell();
  inline zgn_shm* shm();

 protected:
  virtual void RayEnter(uint32_t /*serial*/, VirtualObject* /*virtual_object*/,
      glm::vec3 /*origin*/, glm::vec3 /*direction*/){};

  virtual void RayLeave(
      uint32_t /*serial*/, VirtualObject* /*virtual_object*/){};

  virtual void RayMotion(
      uint32_t /*time*/, glm::vec3 /*origin*/, glm::vec3 /*direction*/){};

  virtual void RayButton(uint32_t /*serial*/, uint32_t /*time*/,
      uint32_t /*button*/, uint32_t /*state*/){};

 private:
  static const struct wl_registry_listener registry_listener_;
  static void GlobalRegistry(void* data, struct wl_registry* registry,
      uint32_t id, const char* interface, uint32_t version);
  static void GlobalRegistryRemove(
      void* data, struct wl_registry* registry, uint32_t id);

  static const struct zgn_seat_listener zgn_seat_listener_;
  static void SeatCapabilities(
      void* data, struct zgn_seat* zgn_seat, uint32_t capabilities);

  static const struct zgn_ray_listener zgn_ray_listener_;
  static void HandleRayEnter(void* data, struct zgn_ray* zgn_ray,
      uint32_t serial, struct zgn_virtual_object* virtual_object,
      struct wl_array* origin, struct wl_array* direction);
  static void HandleRayLeave(void* data, struct zgn_ray* zgn_ray,
      uint32_t serial, struct zgn_virtual_object* virtual_object);
  static void HandleRayMotion(void* data, struct zgn_ray* zgn_ray,
      uint32_t time, struct wl_array* origin, struct wl_array* direction);
  static void HandleRayButton(void* data, struct zgn_ray* zgn_ray,
      uint32_t serial, uint32_t time, uint32_t button, uint32_t state);

  void Poll();

  wl_display* display_ = nullptr;
  wl_registry* registry_ = nullptr;
  zgn_compositor* zgn_compositor_ = nullptr;
  zgn_seat* zgn_seat_ = nullptr;
  zgn_gles_v32* zgn_gles_v32_ = nullptr;
  zgn_shell* zgn_shell_ = nullptr;
  zgn_shm* zgn_shm_ = nullptr;

  zgn_ray* zgn_ray_ = nullptr;  // nullable

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

inline zgn_seat*
Application::seat()
{
  return zgn_seat_;
}

inline zgn_gles_v32*
Application::gles_v32()
{
  return zgn_gles_v32_;
}

inline zgn_shell*
Application::shell()
{
  return zgn_shell_;
}

inline zgn_shm*
Application::shm()
{
  return zgn_shm_;
}

}  // namespace zen::client
