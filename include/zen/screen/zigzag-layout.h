#include <zigzag.h>

#include "zen/screen/output.h"
#include "zen/server.h"

struct zigzag_layout *zn_zigzag_layout_create_default(
    struct zn_output *output, struct zn_server *server);

void zn_zigzag_layout_destroy_default(struct zigzag_layout *self);
