#include <zigzag.h>

#include "zen/screen/output.h"
#include "zen/server.h"

struct zen_zigzag_layout_state {
  struct zn_output *output;
};

void zen_zigzag_layout_on_damage(struct zigzag_node *node);

void zen_zigzag_layout_setup_default(
    struct zigzag_layout *node_layout, struct zn_server *server);
