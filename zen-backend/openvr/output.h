#ifndef ZEN_OPENVR_BACKEND_OUTPUT_H
#define ZEN_OPENVR_BACKEND_OUTPUT_H

#include <libzen-compositor/libzen-compositor.h>

struct zen_output *zen_output_create(struct zen_compositor *compositor);

void zen_output_destroy(struct zen_output *output);

#endif  //  ZEN_OPENVR_BACKEND_OUTPUT_H
