#ifndef LIBZEN_H
#define LIBZEN_H

#include <libzen/debug.h>
#include <libzen/helpers.h>
#include <libzen/timespec-util.h>
#include <time.h>
#include <wayland-server.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zen_compositor;

struct zen_compositor* zen_compositor_create(struct wl_display* display);

void zen_compositor_destroy(struct zen_compositor* compositor);

int zen_compositor_load_backend(struct zen_compositor* compositor);

void zen_compositor_finish_frame(
    struct zen_compositor* compositor, struct timespec next_repaint);

#ifdef __cplusplus
}
#endif

#endif  //  LIBZEN_H
