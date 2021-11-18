#ifndef LIBZEN_OPENGL_RENDERER_H
#define LIBZEN_OPENGL_RENDERER_H

#include <libzen/libzen.h>

struct zen_opengl_renderer;

struct zen_opengl_renderer* zen_opengl_renderer_create(
    struct zen_compositor* compositor);

void zen_opengl_renderer_destroy(struct zen_opengl_renderer* renderer);

void zen_opengl_renderer_set_target();

void zen_opengl_renderer_render(struct zen_opengl_renderer* renderer);

#endif  //  LIBZEN_OPENGL_RENDERER_H
