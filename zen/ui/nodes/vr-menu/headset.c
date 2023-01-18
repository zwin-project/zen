#include "zen/ui/nodes/vr-menu/headset.h"

#include <cairo.h>
#include <zen-common.h>

#include "zen/peer.h"
#include "zen/server.h"
#include "zen/ui/layout-constants.h"
#include "zen/ui/nodes/vr-menu/connect-button.h"
#include "zigzag/cairo-util.h"
#include "zigzag/layout.h"
#include "zigzag/node.h"

static void
zn_vr_menu_item_headset_on_click(struct zigzag_node *node, double x, double y)
{
  UNUSED(node);
  UNUSED(x);
  UNUSED(y);
}

static bool
zn_vr_icon_render(struct zigzag_node *node, cairo_t *cr)
{
  UNUSED(node);
  bool result = zn_cairo_stamp_svg_on_surface(
      cr, VR_ICON_TRANSLUCENT, 0., 0., vr_icon_width, vr_icon_height);
  return result;
}

static bool
zn_vr_menu_item_headset_render(struct zigzag_node *node, cairo_t *cr)
{
  double icon_x = 15.;
  double icon_y = (node->frame.height - vr_icon_height) / 2;
  struct zn_vr_menu_item_headset *self = node->user_data;

  cairo_set_source_surface(cr, self->vr_icon_surface, icon_x, icon_y);
  cairo_paint(cr);

  char hmd_name[8];
  sprintf(hmd_name, "HMD %d", self->idx + 1);

  cairo_set_source_rgba(cr, 0., 0., 0., 0.5);
  cairo_set_font_size(cr, 13);
  struct zn_peer *peer = self->peer;
  zigzag_cairo_draw_text(cr, peer->wired ? "Wired" : peer->host,
      icon_x + vr_icon_width + 57.5, node->frame.height / 2, ZIGZAG_ANCHOR_LEFT,
      ZIGZAG_ANCHOR_CENTER);

  cairo_set_font_face(cr, zn_font_face_get_cairo_font_face(ZN_FONT_BOLD));
  cairo_set_source_rgb(cr, 0., 0., 0.);
  cairo_set_font_size(cr, 14);
  zigzag_cairo_draw_text(cr, hmd_name, icon_x + vr_icon_width + 5.,
      node->frame.height / 2, ZIGZAG_ANCHOR_LEFT, ZIGZAG_ANCHOR_CENTER);

  return true;
}

static const struct zigzag_node_impl implementation = {
    .on_click = zn_vr_menu_item_headset_on_click,
    .render = zn_vr_menu_item_headset_render,
};

static void
zn_vr_menu_item_headset_init_frame(
    struct zigzag_node *node, double screen_width, double screen_height)
{
  struct zn_vr_menu_item_headset *self = node->user_data;
  node->pending.frame.x = screen_width - vr_menu_bubble_width -
                          vr_menu_space_right +
                          (vr_menu_bubble_width - vr_menu_headset_width) / 2;
  struct zn_server *server = zn_server_get_singleton();
  struct zn_remote *remote = server->remote;
  node->pending.frame.y =
      screen_height - menu_bar_height - tip_height - vr_how_to_connect_height -
      (wl_list_length(&remote->peer_list) - self->idx) * vr_menu_headset_height;
  node->pending.frame.width = vr_menu_headset_width;
  node->pending.frame.height = vr_menu_headset_height;
}

struct zn_vr_menu_item_headset *
zn_vr_menu_item_headset_create(struct zigzag_layout *zigzag_layout,
    struct wlr_renderer *renderer, struct zn_peer *peer, int idx)
{
  struct zn_vr_menu_item_headset *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->peer = peer;
  self->idx = idx;

  struct zigzag_node *zigzag_node =
      zigzag_node_create(&implementation, zigzag_layout, true, self);

  if (zigzag_node == NULL) {
    zn_error("Failed to create a zigzag_node");
    goto err_vr_menu_item_headset;
  }
  self->zigzag_node = zigzag_node;

  self->connect_button =
      zn_vr_menu_headset_connect_button_create(zigzag_layout, peer, idx);

  if (self->connect_button == NULL) {
    zn_error("Failed to create the connect button");
    goto err_zigzag_node;
  }

  zigzag_node_add_child(
      self->zigzag_node, self->connect_button->zigzag_node, renderer);

  self->vr_icon_surface = zigzag_node_render_cairo_surface(
      zigzag_node, zn_vr_icon_render, vr_icon_width, vr_icon_height);

  if (self->vr_icon_surface == NULL) {
    zn_error("Failed to load the icon");
    goto err_connect_button;
  }
  zn_vr_menu_item_headset_init_frame(self->zigzag_node,
      self->zigzag_node->layout->screen_width,
      self->zigzag_node->layout->screen_height);

  wl_list_init(&self->link);

  return self;

err_connect_button:
  zn_vr_menu_headset_connect_button_destroy(self->connect_button);

err_zigzag_node:
  zigzag_node_destroy(self->zigzag_node);

err_vr_menu_item_headset:
  free(self);

err:
  return NULL;
}

void
zn_vr_menu_item_headset_destroy(struct zn_vr_menu_item_headset *self)
{
  wl_list_remove(&self->link);
  cairo_surface_destroy(self->vr_icon_surface);
  zn_vr_menu_headset_connect_button_destroy(self->connect_button);
  zigzag_node_destroy(self->zigzag_node);
  free(self);
}
