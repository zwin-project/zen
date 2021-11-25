#ifndef ZEN_RENDERER_OPENGL_H
#define ZEN_RENDERER_OPENGL_H

#include <libzen-compositor/libzen-compositor.h>

struct zen_opengl {
  struct zen_compositor* compositor;
  struct wl_global* global;
};

struct zen_opengl* zen_opengl_create(struct zen_compositor* compositor);

void zen_opengl_destroy(struct zen_opengl* opengl);

#endif  //  ZEN_RENDERER_OPENGL_H
