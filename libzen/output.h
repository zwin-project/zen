#ifndef LIBZEN_OUTPUT_H
#define LIBZEN_OUTPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <libzen/libzen.h>
#include <time.h>

struct zen_output {
  struct zen_compositor *compositor;

  uint32_t refresh;
};

struct zen_output *zen_output_create(struct zen_compositor *compositor);

void zen_output_destroy(struct zen_output *output);

#ifdef __cplusplus
}
#endif

#endif  //  LIBZEN_OUTPUT_H
