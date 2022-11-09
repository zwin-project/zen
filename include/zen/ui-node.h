// screen has one zen_ui_node
//
struct zen_ui_node {
  struct wlr_box *frame;
  struct wlr_texture *texture;
  void (*callback)(struct zen_ui_node *self, double x, double y);
  struct wl_list *link;
};