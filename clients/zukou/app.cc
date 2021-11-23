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
shm_format(void *data, struct wl_shm *wl_shm, uint32_t format)
{
  App *app = (App *)data;
  app->ShmFormat(wl_shm, format);
}

static const struct wl_shm_listener shm_listener = {
    shm_format,
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

  if (compositor_ == nullptr || shm_ == nullptr || opengl_ == nullptr)
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
  } else if (strcmp(interface, "wl_shm") == 0) {
    shm_ = (wl_shm *)wl_registry_bind(registry, id, &wl_shm_interface, version);
  } else if (strcmp(interface, "zgn_opengl") == 0) {
    opengl_ = (zgn_opengl *)wl_registry_bind(
        registry, id, &zgn_opengl_interface, version);
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
