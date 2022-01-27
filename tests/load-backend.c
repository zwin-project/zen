#include <assert.h>
#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>

#include "test-runner.h"

struct zen_config config = {
    .fullscreen_preview = false,
    .seat = "seat0",
};

TEST(load_backend)
{
  struct wl_display *display;
  struct zen_compositor *compositor;

  display = wl_display_create();
  compositor = zen_compositor_create(display, &config);
  assert(compositor->backend == NULL);

  zen_compositor_load_shell(compositor);
  zen_compositor_load_renderer(compositor);
  zen_compositor_load_backend(compositor);
  assert(compositor->backend != NULL);
}
