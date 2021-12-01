#include <errno.h>
#include <string.h>
#include <zigen-client-protocol.h>
#include <zigen-opengl-client-protocol.h>
#include <zukou.h>

namespace zukou {
App::App() {}

App::~App()
{
  if (display_) wl_display_disconnect(display_);
}

static void
global_registry_handler(void *data, struct wl_registry *registry, uint32_t id,
    const char *interface, uint32_t version)
{
  App *app = (App *)data;
  app->GlobalRegistryHandler(registry, id, interface, version);
}

static void
global_registry_remover(void *data, struct wl_registry *registry, uint32_t id)
{
  App *app = (App *)data;
  app->GlobalRegistryRemover(registry, id);
}

static const struct wl_registry_listener registry_listener = {
    global_registry_handler,
    global_registry_remover,
};

static void
seat_capabilities(void *data, struct zgn_seat *seat, uint32_t capability)
{
  App *app = (App *)data;
  app->Capabilities(seat, capability);
}

static const struct zgn_seat_listener seat_listener = {
    seat_capabilities,
};

static void
shm_format(void *data, struct wl_shm *wl_shm, uint32_t format)
{
  App *app = (App *)data;
  app->ShmFormat(wl_shm, format);
}

static const struct wl_shm_listener shm_listener = {
    shm_format,
};

static void
ray_enter(void *data, struct zgn_ray *ray, uint32_t serial,
    struct zgn_virtual_object *virtual_object, struct wl_array *origin,
    struct wl_array *direction)
{
  App *app = (App *)data;
  glm::vec3 origin_vec = glm_vec3_from_wl_array(origin);
  glm::vec3 direction_vec = glm_vec3_from_wl_array(direction);
  app->RayEnter(ray, serial, virtual_object, origin_vec, direction_vec);
}

static void
ray_leave(void *data, struct zgn_ray *ray, uint32_t serial,
    struct zgn_virtual_object *virtual_object)
{
  App *app = (App *)data;
  app->RayLeave(ray, serial, virtual_object);
}

static void
ray_motion(void *data, struct zgn_ray *ray, struct wl_array *origin,
    struct wl_array *direction)
{
  App *app = (App *)data;
  glm::vec3 origin_vec = glm_vec3_from_wl_array(origin);
  glm::vec3 direction_vec = glm_vec3_from_wl_array(direction);
  app->RayMotion(ray, origin_vec, direction_vec);
}

static void
ray_button(void *data, struct zgn_ray *ray, uint32_t serial, uint32_t time,
    uint32_t button, uint32_t state)
{
  App *app = (App *)data;
  app->RayButton(ray, serial, time, button, (enum zgn_ray_button_state)state);
}

static const struct zgn_ray_listener ray_listener = {
    ray_enter,
    ray_leave,
    ray_motion,
    ray_button,
};

bool
App::Connect(const char *socket)
{
  display_ = wl_display_connect(socket);
  if (display_ == nullptr) goto err;

  registry_ = wl_display_get_registry(display_);
  if (registry_ == nullptr) goto err_registry;

  wl_registry_add_listener(registry_, &registry_listener, this);

  wl_display_dispatch(display_);
  wl_display_roundtrip(display_);

  if (compositor_ == nullptr || shm_ == nullptr || opengl_ == nullptr ||
      shell_ == nullptr)
    goto err_globals;

  return true;

err_globals:
  wl_registry_destroy(registry_);

err_registry:
  wl_display_disconnect(display_);
  display_ = NULL;

err:
  return false;
}

void
App::GlobalRegistryHandler(struct wl_registry *registry, uint32_t id,
    const char *interface, uint32_t version)
{
  if (strcmp(interface, "zgn_compositor") == 0) {
    compositor_ = (zgn_compositor *)wl_registry_bind(
        registry, id, &zgn_compositor_interface, version);
  } else if (strcmp(interface, "zgn_seat") == 0) {
    seat_ = (zgn_seat *)wl_registry_bind(
        registry, id, &zgn_seat_interface, version);
    zgn_seat_add_listener(seat_, &seat_listener, this);
  } else if (strcmp(interface, "wl_shm") == 0) {
    shm_ = (wl_shm *)wl_registry_bind(registry, id, &wl_shm_interface, version);
    wl_shm_add_listener(shm_, &shm_listener, this);
  } else if (strcmp(interface, "zgn_opengl") == 0) {
    opengl_ = (zgn_opengl *)wl_registry_bind(
        registry, id, &zgn_opengl_interface, version);
  } else if (strcmp(interface, "zgn_shell") == 0) {
    shell_ = (zgn_shell *)wl_registry_bind(
        registry, id, &zgn_shell_interface, version);
  }
}

void
App::GlobalRegistryRemover(struct wl_registry *registry, uint32_t id)
{
  (void)registry;
  (void)id;
}

void
App::ShmFormat(struct wl_shm *wl_shm, uint32_t format)
{
  (void)wl_shm;
  (void)format;
}

void
App::Capabilities(struct zgn_seat *seat, uint32_t capability)
{
  if (capability & ZGN_SEAT_CAPABILITY_RAY && ray_ == NULL) {
    ray_ = zgn_seat_get_ray(seat);
    zgn_ray_add_listener(ray_, &ray_listener, this);
  }

  if (!(capability & ZGN_SEAT_CAPABILITY_RAY) && ray_) {
    zgn_ray_release(ray_);
    ray_ = NULL;
  }
}

void
App::RayEnter(struct zgn_ray *ray, uint32_t serial,
    struct zgn_virtual_object *virtual_object, glm::vec3 origin,
    glm::vec3 direction)
{
  (void)ray;
  (void)serial;
  (void)virtual_object;
  (void)origin;
  (void)direction;
}

void
App::RayLeave(struct zgn_ray *ray, uint32_t serial,
    struct zgn_virtual_object *virtual_object)
{
  (void)ray;
  (void)serial;
  (void)virtual_object;
}

void
App::RayMotion(struct zgn_ray *ray, glm::vec3 origin, glm::vec3 direction)
{
  (void)ray;
  (void)origin;
  (void)direction;
}

void
App::RayButton(struct zgn_ray *ray, uint32_t serial, uint32_t time,
    uint32_t button, enum zgn_ray_button_state state)
{
  (void)ray;
  (void)serial;
  (void)time;
  (void)button;
  (void)state;
}

bool
App::Run()
{
  int ret;
  running_ = true;
  while (running_) {
    while (wl_display_prepare_read(display()) != 0) {
      if (errno != EAGAIN) return false;
      wl_display_dispatch_pending(display());
    }
    ret = wl_display_flush(display());
    if (ret == -1) return false;
    wl_display_read_events(display());
    wl_display_dispatch_pending(display());
  }
  return true;
}

void
App::Terminate()
{
  running_ = false;
}

}  // namespace zukou
