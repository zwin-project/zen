#include "compositor.h"

#include <libzen/libzen.h>
#include <stdio.h>
#include <wayland-server.h>

#include "backend.h"

static int
repaint_timer_handler(void *data)
{
  struct zen_compositor *compositor = data;
  struct zen_output *output = compositor->backend->output;
  output->repaint(output);
  // TODO: send frame callback_done here
  return 0;
}

WL_EXPORT int
zen_compositor_load_backend(struct zen_compositor *compositor)
{
  struct zen_backend *backend;

  backend = zen_backend_create(compositor);
  if (backend == NULL) {
    zen_log("compositor: failed to create a backend\n");
    goto err;
  }

  compositor->backend = backend;

  return 0;

err:
  return -1;
}

WL_EXPORT struct zen_compositor *
zen_compositor_create(struct wl_display *display)
{
  struct zen_compositor *compositor;
  struct wl_event_loop *loop;
  struct wl_event_source *repaint_timer;
  struct wl_global *global;
  UNUSED(global);

  compositor = zalloc(sizeof *compositor);
  if (compositor == NULL) {
    zen_log("compositor: failed to allcate memory\n");
    goto out;
  }

  loop = wl_display_get_event_loop(display);
  repaint_timer =
      wl_event_loop_add_timer(loop, repaint_timer_handler, compositor);

  compositor->display = display;
  compositor->backend = NULL;
  compositor->repaint_timer = repaint_timer;
  compositor->repaint_window_msec = DEFAULT_REPAINT_WINDOW;

  return compositor;

out:
  return NULL;
}

WL_EXPORT void
zen_compositor_destroy(struct zen_compositor *compositor)
{
  if (compositor->backend) zen_backend_destroy(compositor->backend);
  wl_event_source_remove(compositor->repaint_timer);
  free(compositor);
}

WL_EXPORT void
zen_compositor_complete_frame(struct zen_compositor *compositor,
                              struct timespec next_repaint)
{
  struct timespec now;
  int64_t next_msec;

  if (timespec_get(&now, TIME_UTC) < 0)
    zen_log("compositor: [WARNING] failed to get current time\n");
  next_msec = timespec_sub_to_msec(&next_repaint, &now);
  next_msec -= compositor->repaint_window_msec;

  if (next_msec < 1) next_msec = 1;

  wl_event_source_timer_update(compositor->repaint_timer, next_msec);
}
