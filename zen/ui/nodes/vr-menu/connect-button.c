#include "zen/ui/nodes/vr-menu/connect-button.h"

#include <cairo.h>
#include <zen-common.h>

#include "zen/peer.h"
#include "zen/server.h"
#include "zen/ui/layout-constants.h"
#include "zen/ui/nodes/vr-menu/vr-menu.h"
#include "zigzag/cairo-util.h"
#include "zigzag/layout.h"
#include "zigzag/node.h"

static void
zn_vr_menu_headset_connect_button_on_click(
    struct zigzag_node *node, double x, double y)
{
  UNUSED(x);
  UNUSED(y);
  struct zn_vr_menu_headset_connect_button *self = node->user_data;
  struct zn_peer *peer = self->peer;
  struct zn_server *server = zn_server_get_singleton();
  struct zn_remote *remote = server->remote;
  struct znr_session *session =
      znr_remote_create_session(remote->znr_remote, peer->znr_remote_peer);
  if (session == NULL) return;

  zn_peer_set_session(peer, session);
  zna_system_set_current_session(server->appearance_system, session);
}

static bool
zn_vr_menu_headset_connect_button_render(struct zigzag_node *node, cairo_t *cr)
{
  cairo_set_source_rgb(cr, 0.07, 0.12, 0.30);
  zn_cairo_draw_rounded_rectangle(cr, 0., 0., node->frame.width,
      node->frame.height, node->frame.height / 2);
  cairo_fill(cr);

  cairo_set_font_size(cr, 13);
  cairo_set_source_rgb(cr, 1., 1., 1.);
  zn_cairo_draw_text(cr, "Connect", node->frame.width / 2,
      node->frame.height / 2, ZN_CAIRO_ANCHOR_CENTER, ZN_CAIRO_ANCHOR_CENTER);
  return true;
}

static const struct zigzag_node_impl implementation = {
    .on_click = zn_vr_menu_headset_connect_button_on_click,
    .render = zn_vr_menu_headset_connect_button_render,
};

static void
zn_vr_menu_headset_connect_button_init_frame(
    struct zigzag_node *node, double screen_width, double screen_height)
{
  struct zn_vr_menu_headset_connect_button *self = node->user_data;
  node->pending.frame.x = screen_width - vr_menu_space_right -
                          (vr_menu_bubble_width - vr_menu_headset_width) / 2 -
                          vr_menu_headset_connect_button_margin -
                          vr_menu_headset_connect_button_width;

  struct zn_server *server = zn_server_get_singleton();
  struct zn_remote *remote = server->remote;

  node->pending.frame.y =
      screen_height - menu_bar_height - tip_height - vr_how_to_connect_height -
      (wl_list_length(&remote->peer_list) - self->idx) *
          vr_menu_headset_height +
      (vr_menu_headset_height - vr_menu_headset_connect_button_height) / 2;

  node->pending.frame.width = vr_menu_headset_connect_button_width;
  node->pending.frame.height = vr_menu_headset_connect_button_height;
}

struct zn_vr_menu_headset_connect_button *
zn_vr_menu_headset_connect_button_create(
    struct zigzag_layout *zigzag_layout, struct zn_peer *peer, int idx)
{
  struct zn_vr_menu_headset_connect_button *self;

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
    goto err_vr_menu_headset_connect_button;
  }
  self->zigzag_node = zigzag_node;

  zn_vr_menu_headset_connect_button_init_frame(self->zigzag_node,
      self->zigzag_node->layout->screen_width,
      self->zigzag_node->layout->screen_height);

  return self;

err_vr_menu_headset_connect_button:
  free(self);

err:
  return NULL;
}

void
zn_vr_menu_headset_connect_button_destroy(
    struct zn_vr_menu_headset_connect_button *self)
{
  zigzag_node_destroy(self->zigzag_node);
  free(self);
}
