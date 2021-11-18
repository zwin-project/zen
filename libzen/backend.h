#ifndef LIBZEN_BACKEND_H
#define LIBZEN_BACKEND_H

#include "compositor.h"
#include "output.h"

#ifdef __cplusplus
extern "C" {
#endif

struct zen_backend {
  struct zen_output* output;
};

struct zen_backend* zen_backend_create(struct zen_compositor* compositor);

void zen_backend_destroy(struct zen_backend* backend);

#ifdef __cplusplus
}
#endif

#endif  //  LIBZEN_BACKEND_H
