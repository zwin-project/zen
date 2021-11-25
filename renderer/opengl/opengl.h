#ifndef LIBZEN_OPENGL_H
#define LIBZEN_OPENGL_H

#include <libzen/libzen.h>

struct zen_opengl {
  struct zen_compositor* compositor;
  struct wl_global* global;
};

struct zen_opengl* zen_opengl_create(struct zen_compositor* compositor);

void zen_opengl_destroy(struct zen_opengl* opengl);

#endif  //  LIBZEN_OPENGL_H
