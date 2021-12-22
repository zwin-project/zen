#ifndef ZEN_OPENVR_BACKEND_OUTPUT_H
#define ZEN_OPENVR_BACKEND_OUTPUT_H

#include <libzen-compositor/libzen-compositor.h>

#include "gl-window.h"
#include "hmd.h"

struct openvr_output {
  struct zen_output base;
  struct zen_compositor* compositor;
  struct zen_opengl_renderer* renderer;

  GlWindow* window;
  Hmd* hmd;
};

struct zen_output* zen_output_create(struct zen_compositor* compositor);

void zen_output_destroy(struct zen_output* output);

#endif  //  ZEN_OPENVR_BACKEND_OUTPUT_H
