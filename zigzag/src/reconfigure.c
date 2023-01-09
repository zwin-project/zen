#include <zen-common.h>
#include <zigzag.h>

struct zigzag_reconfigure_context {
  enum zigzag_reconfigure_direction direction;
  struct zigzag_node *parent_node;
  struct zigzag_node *prev_node;
  int child_list_length;
  double child_total_size;
  int index;
};

static void
zigzag_node_reconfigure_start(
    struct zigzag_node *self, struct zigzag_reconfigure_context *ctx)
{
  if (ctx->direction == ZIGZAG_RECONFIGURE_HORIZONTAL) {
    if (!ctx->prev_node) {
      self->pending.frame.x = ctx->parent_node->frame.x +
                              ctx->parent_node->padding.left +
                              self->margin.left;
    } else {
      self->pending.frame.x = ctx->prev_node->frame.x +
                              ctx->prev_node->frame.width +
                              ctx->prev_node->margin.right + self->margin.left;
    }
  } else {
    if (!ctx->prev_node) {
      self->pending.frame.y = ctx->parent_node->frame.y +
                              ctx->parent_node->padding.top + self->margin.top;
    } else {
      self->pending.frame.y = ctx->prev_node->frame.y +
                              ctx->prev_node->frame.height +
                              ctx->prev_node->margin.bottom + self->margin.top;
    }
  }
}

static void
zigzag_node_reconfigure_center(
    struct zigzag_node *self, struct zigzag_reconfigure_context *ctx)
{
  if (ctx->direction == ZIGZAG_RECONFIGURE_HORIZONTAL) {
    if (!ctx->prev_node) {
      self->pending.frame.x =
          ctx->parent_node->frame.x +
          (ctx->parent_node->frame.width - ctx->child_total_size) / 2 +
          self->margin.left;
    } else {
      self->pending.frame.x = ctx->prev_node->frame.x +
                              ctx->prev_node->frame.width +
                              ctx->prev_node->margin.right + self->margin.left;
    }
  } else {
    if (!ctx->prev_node) {
      self->pending.frame.y =
          ctx->parent_node->frame.y +
          (ctx->parent_node->frame.height - ctx->child_total_size) / 2 +
          self->margin.top;
    } else {
      self->pending.frame.y = ctx->prev_node->frame.y +
                              ctx->prev_node->frame.height +
                              ctx->prev_node->margin.bottom + self->margin.top;
    }
  }
}

static void
zigzag_node_reconfigure_end(
    struct zigzag_node *self, struct zigzag_reconfigure_context *ctx)
{
  if (ctx->direction == ZIGZAG_RECONFIGURE_HORIZONTAL) {
    if (!ctx->prev_node) {
      self->pending.frame.x = ctx->parent_node->frame.x +
                              ctx->parent_node->frame.width -
                              ctx->parent_node->padding.right -
                              self->margin.right - ctx->child_total_size;
    } else {
      self->pending.frame.x = ctx->prev_node->frame.x +
                              ctx->prev_node->frame.width +
                              ctx->prev_node->margin.right + self->margin.left;
    }
  } else {
    if (!ctx->prev_node) {
      self->pending.frame.y = ctx->parent_node->frame.y +
                              ctx->parent_node->frame.height -
                              ctx->parent_node->padding.bottom -
                              self->margin.bottom - ctx->child_total_size;
    } else {
      self->pending.frame.y = ctx->prev_node->frame.y +
                              ctx->prev_node->frame.height +
                              ctx->prev_node->margin.bottom + self->margin.top;
    }
  }
}

static void
zigzag_node_reconfigure_justify(
    struct zigzag_node *self, struct zigzag_reconfigure_context *ctx)
{
  if (ctx->direction == ZIGZAG_RECONFIGURE_HORIZONTAL) {
    const double space =
        (ctx->parent_node->frame.width - ctx->child_total_size) /
        (ctx->child_list_length - 1);

    if (!ctx->prev_node) {
      self->pending.frame.x = ctx->parent_node->frame.x +
                              ctx->parent_node->padding.left +
                              self->margin.left;
    } else if (ctx->index == ctx->child_list_length - 1) {
      self->pending.frame.x = ctx->parent_node->frame.x +
                              ctx->parent_node->frame.width -
                              ctx->parent_node->padding.right -
                              self->frame.width - self->margin.right;
    } else {
      self->pending.frame.x =
          ctx->prev_node->frame.x + ctx->prev_node->frame.width +
          ctx->prev_node->margin.right + space + self->margin.left;
    }
  } else {
    const double space =
        (ctx->parent_node->frame.height - ctx->child_total_size) /
        (ctx->child_list_length - 1);

    if (!ctx->prev_node) {
      self->pending.frame.y = ctx->parent_node->frame.y +
                              ctx->parent_node->padding.top + self->margin.top;
    } else if (ctx->index == ctx->child_list_length - 1) {
      self->pending.frame.y = ctx->parent_node->frame.y +
                              ctx->parent_node->frame.height -
                              ctx->parent_node->padding.bottom -
                              self->frame.height - self->margin.bottom;
    } else {
      self->pending.frame.y =
          ctx->prev_node->frame.y + ctx->prev_node->frame.height +
          ctx->prev_node->margin.bottom + space + self->margin.top;
    }
  }
}

static void (*reconfigure[])(
    struct zigzag_node *self, struct zigzag_reconfigure_context *ctx) = {
    [ZIGZAG_RECONFIGURE_START] = zigzag_node_reconfigure_start,
    [ZIGZAG_RECONFIGURE_END] = zigzag_node_reconfigure_end,
    [ZIGZAG_RECONFIGURE_CENTER] = zigzag_node_reconfigure_center,
    [ZIGZAG_RECONFIGURE_JUSTIFY] = zigzag_node_reconfigure_justify,
};

void
zigzag_node_reconfigure(struct zigzag_node *parent,
    enum zigzag_reconfigure_direction direction,
    enum zigzag_reconfigure_type type)
{
  UNUSED(type);

  if (!zn_assert(!wl_list_empty(&parent->node_list), "node_list is empty")) {
    return;
  }

  double width, height;
  zigzag_node_child_total_size(parent, &width, &height);

  struct zigzag_reconfigure_context ctx = {
      .direction = direction,
      .parent_node = parent,
      .child_list_length = wl_list_length(&parent->node_list),
      .child_total_size =
          (direction == ZIGZAG_RECONFIGURE_HORIZONTAL) ? width : height,
      .prev_node = NULL,
      .index = 0,
  };

  UNUSED(reconfigure);
  struct zigzag_node *node_iter;
  wl_list_for_each_reverse (node_iter, &parent->node_list, link) {
    reconfigure[type](node_iter, &ctx);
    zigzag_node_update_frame(node_iter);
    ++ctx.index;
    ctx.prev_node = node_iter;
  }
}
