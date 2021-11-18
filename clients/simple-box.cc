#include <wayland-client.h>

#include <iostream>

static void
global_registry_handler(void *data, struct wl_registry *registry, uint32_t id,
                        const char *interface, uint32_t version)
{
  (void)data;
  (void)registry;
  (void)id;
  (void)version;
  (void)interface;
}

static void
global_registry_remover(void *data, struct wl_registry *registry, uint32_t id)
{
  (void)data;
  (void)registry;
  (void)id;
}

static const struct wl_registry_listener registry_listener = {
    global_registry_handler,
    global_registry_remover,
};

int
main()
{
  struct wl_display *display;
  struct wl_registry *registry;

  display = wl_display_connect("zigen-0");

  registry = wl_display_get_registry(display);

  wl_registry_add_listener(registry, &registry_listener, nullptr);

  wl_display_flush(display);

  return 0;
}
