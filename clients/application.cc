#include "application.h"

#include <errno.h>

#include <cstring>
#include <iostream>

#include "array.h"

namespace zen::client {

void
Application::GlobalRegistry(void* data, struct wl_registry* registry,
    uint32_t id, const char* interface, uint32_t version)
{
  auto app = static_cast<Application*>(data);
  if (std::strcmp(interface, "zgn_compositor") == 0) {
    app->zgn_compositor_ = static_cast<zgn_compositor*>(
        wl_registry_bind(registry, id, &zgn_compositor_interface, version));
  } else if (std::strcmp(interface, "zgn_seat") == 0) {
    app->zgn_seat_ = static_cast<zgn_seat*>(
        wl_registry_bind(registry, id, &zgn_seat_interface, version));
    zgn_seat_add_listener(
        app->zgn_seat_, &Application::zgn_seat_listener_, app);
  } else if (std::strcmp(interface, "zgn_gles_v32") == 0) {
    app->zgn_gles_v32_ = static_cast<zgn_gles_v32*>(
        wl_registry_bind(registry, id, &zgn_gles_v32_interface, version));
  } else if (std::strcmp(interface, "zgn_shell") == 0) {
    app->zgn_shell_ = static_cast<zgn_shell*>(
        wl_registry_bind(registry, id, &zgn_shell_interface, version));
  } else if (std::strcmp(interface, "zgn_shm") == 0) {
    app->zgn_shm_ = static_cast<zgn_shm*>(
        wl_registry_bind(registry, id, &zgn_shm_interface, version));
  }
}

void
Application::GlobalRegistryRemove(
    void* /*data*/, struct wl_registry* /*registry*/, uint32_t /*id*/)
{}

void
Application::SeatCapabilities(
    void* data, struct zgn_seat* /*zgn_seat*/, uint32_t capabilities)
{
  auto app = static_cast<Application*>(data);
  bool has_ray =
      (ZGN_SEAT_CAPABILITY_RAY_ORIGIN | ZGN_SEAT_CAPABILITY_RAY_DIRECTION) &
      capabilities;

  if (has_ray && !app->zgn_ray_) {
    app->zgn_ray_ = zgn_seat_get_ray(app->zgn_seat_);
    zgn_ray_add_listener(app->zgn_ray_, &Application::zgn_ray_listener_, app);
  }

  if (!has_ray && app->zgn_ray_) {
    zgn_ray_release(app->zgn_ray_);
    app->zgn_ray_ = nullptr;
  }
}

void
Application::HandleRayEnter(void* data, struct zgn_ray* /*zgn_ray*/,
    uint32_t serial, struct zgn_virtual_object* virtual_object_proxy,
    struct wl_array* origin_array, struct wl_array* direction_array)
{
  auto virtual_object = static_cast<VirtualObject*>(
      wl_proxy_get_user_data((wl_proxy*)virtual_object_proxy));
  auto app = static_cast<Application*>(data);
  glm::vec3 origin, direction;
  to_vec3(origin_array, origin);
  to_vec3(direction_array, direction);

  app->RayEnter(serial, virtual_object, origin, direction);
}

void
Application::HandleRayLeave(void* data, struct zgn_ray* /*zgn_ray*/,
    uint32_t serial, struct zgn_virtual_object* virtual_object_proxy)
{
  auto app = static_cast<Application*>(data);
  auto virtual_object = static_cast<VirtualObject*>(
      wl_proxy_get_user_data((wl_proxy*)virtual_object_proxy));
  app->RayLeave(serial, virtual_object);
}

void
Application::HandleRayMotion(void* data, struct zgn_ray* /*zgn_ray*/,
    uint32_t time, struct wl_array* origin_array,
    struct wl_array* direction_array)
{
  auto app = static_cast<Application*>(data);
  glm::vec3 origin, direction;
  to_vec3(origin_array, origin);
  to_vec3(direction_array, direction);

  app->RayMotion(time, origin, direction);
}

void
Application::HandleRayButton(void* data, struct zgn_ray* /*zgn_ray*/,
    uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
  auto app = static_cast<Application*>(data);
  app->RayButton(serial, time, button, state);
}

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

  if (zgn_compositor_ == nullptr || zgn_seat_ == nullptr ||
      zgn_gles_v32_ == nullptr || zgn_shell_ == nullptr ||
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
  if (zgn_shm_) zgn_shm_destroy(zgn_shm_);
  zgn_shm_ = nullptr;

  if (zgn_shell_) zgn_shell_destroy(zgn_shell_);
  zgn_shell_ = nullptr;

  if (zgn_gles_v32_) zgn_gles_v32_destroy(zgn_gles_v32_);
  zgn_gles_v32_ = nullptr;

  if (zgn_seat_) zgn_seat_destroy(zgn_seat_);
  zgn_seat_ = nullptr;

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
  if (zgn_shell_) zgn_shell_destroy(zgn_shell_);
  if (zgn_gles_v32_) zgn_gles_v32_destroy(zgn_gles_v32_);
  if (zgn_seat_) zgn_seat_destroy(zgn_seat_);
  if (zgn_compositor_) zgn_compositor_destroy(zgn_compositor_);
  if (registry_) wl_registry_destroy(registry_);
  if (display_) wl_display_disconnect(display_);
}

const struct wl_registry_listener Application::registry_listener_ = {
    Application::GlobalRegistry,
    Application::GlobalRegistryRemove,
};

const struct zgn_seat_listener Application::zgn_seat_listener_ = {
    Application::SeatCapabilities,
};

const struct zgn_ray_listener Application::zgn_ray_listener_ = {
    Application::HandleRayEnter,
    Application::HandleRayLeave,
    Application::HandleRayMotion,
    Application::HandleRayButton,
};

}  // namespace zen::client
