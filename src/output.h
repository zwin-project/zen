#ifndef ZEN_OUTPUT_H
#define ZEN_OUTPUT_H

#include <wlr/types/wlr_output.h>

#include "server.h"

/** self-deleting object */
struct zn_output;

struct zn_output *zn_output_create(
    struct wlr_output *wlr_output, struct zn_server *server);

#endif  //  ZEN_OUTPUT_H
