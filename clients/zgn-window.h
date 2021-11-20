#ifndef ZGN_CLIENT_WINDOW_H
#define ZGN_CLIENT_WINDOW_H

#include <errno.h>
#include <string.h>
#include <wayland-client.h>
#include <zigen-client-protocol.h>

template <class T>
class ZgnWindow
{
 public:
  ZgnWindow(T *delegate);
  ~ZgnWindow();
  bool Connect(const char *socket);
  void GlobalRegistryHandler(struct wl_registry *registry, uint32_t id,
      const char *interface, uint32_t version);
  void GlobalRegistryRemover(struct wl_registry *registry, uint32_t id);
  void ShmFormat(struct wl_shm *wl_shm, uint32_t format);
  bool Run();
  inline struct wl_display *display();
  inline struct wl_registry *registry();
  inline struct zgn_compositor *compositor();
  inline struct wl_shm *shm();

 private:
  T *delegate_;
  struct wl_display *display_;
  struct wl_registry *registry_;
  struct zgn_compositor *compositor_;
  struct wl_shm *shm_;
};

template <class T>
static void
global_registry_handler(void *data, struct wl_registry *registry, uint32_t id,
    const char *interface, uint32_t version)
{
  ZgnWindow<T> *zgn_window = (ZgnWindow<T> *)data;
  zgn_window->GlobalRegistryHandler(registry, id, interface, version);
}

template <class T>
static void
global_registry_remover(void *data, struct wl_registry *registry, uint32_t id)
{
  ZgnWindow<T> *zgn_window = (ZgnWindow<T> *)data;
  zgn_window->GlobalRegistryRemover(registry, id);
}

template <class T>
static const struct wl_registry_listener registry_listener = {
    global_registry_handler<T>,
    global_registry_remover<T>,
};

template <class T>
static void
shm_format(void *data, struct wl_shm *wl_shm, uint32_t format)
{
  ZgnWindow<T> *zgn_window = (ZgnWindow<T> *)data;
  zgn_window->ShmFormat(wl_shm, format);
}

template <class T>
static const struct wl_shm_listener shm_listener = {
    shm_format<T>,
};

template <class T>
ZgnWindow<T>::ZgnWindow(T *delegate)
{
  delegate_ = delegate;
  display_ = nullptr;
}

template <class T>
ZgnWindow<T>::~ZgnWindow()
{
  if (display_) wl_display_disconnect(display_);
}

template <class T>
bool
ZgnWindow<T>::Connect(const char *socket)
{
  display_ = wl_display_connect(socket);
  if (display_ == nullptr) goto err;

  registry_ = wl_display_get_registry(display_);
  if (registry_ == nullptr) goto err_registry;

  wl_registry_add_listener(registry_, &registry_listener<T>, this);

  wl_display_dispatch(display_);
  wl_display_roundtrip(display_);

  if (compositor_ == nullptr || shm_ == nullptr) goto err_globals;

  return true;

err_globals:
  wl_registry_destroy(registry_);

err_registry:
  wl_display_disconnect(display_);

err:
  return false;
}

template <class T>
void
ZgnWindow<T>::GlobalRegistryHandler(struct wl_registry *registry, uint32_t id,
    const char *interface, uint32_t version)
{
  if (strcmp(interface, "zgn_compositor") == 0) {
    compositor_ = (zgn_compositor *)wl_registry_bind(
        registry, id, &zgn_compositor_interface, version);
  } else if (strcmp(interface, "wl_shm") == 0) {
    shm_ = (wl_shm *)wl_registry_bind(registry, id, &wl_shm_interface, version);
  }
}

template <class T>
void
ZgnWindow<T>::GlobalRegistryRemover(struct wl_registry *registry, uint32_t id)
{
  (void)registry;
  (void)id;
}

template <class T>
void
ZgnWindow<T>::ShmFormat(struct wl_shm *wl_shm, uint32_t format)
{
  (void)wl_shm;
  (void)format;
}

template <class T>
bool
ZgnWindow<T>::Run()
{
  int ret;
  while (1) {
    while (wl_display_prepare_read(display()) != 0) {
      if (errno != EAGAIN) return false;
      wl_display_dispatch_pending(display());
    }
    ret = wl_display_flush(display());
    if (ret == -1) return false;
    wl_display_read_events(display());
    wl_display_dispatch_pending(display());
  }
}

template <class T>
inline struct wl_display *
ZgnWindow<T>::display()
{
  return display_;
}

template <class T>
inline struct wl_registry *
ZgnWindow<T>::registry()
{
  return registry_;
}

template <class T>
inline struct zgn_compositor *
ZgnWindow<T>::compositor()
{
  return compositor_;
}

template <class T>
inline struct wl_shm *
ZgnWindow<T>::shm()
{
  return shm_;
}

#endif  //  ZGN_CLIENT_WINDOW_H
