#include <stdbool.h>
#include <zigzag.h>

struct zn_board_node {
  struct zigzag_node *zigzag_node;
  bool left;
  bool right;
};

struct zn_board_node *zn_board_node_create(
    struct zigzag_layout *zigzag_layout, bool left, bool right);

void zn_board_node_destroy(struct zn_board_node *self);
