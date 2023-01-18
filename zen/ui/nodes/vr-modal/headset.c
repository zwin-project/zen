#include "zen/ui/nodes/vr-modal/headset.h"

#include <cairo.h>
#include <string.h>
#include <zen-common.h>

#include "zen/peer.h"
#include "zen/screen/output.h"
#include "zen/server.h"
#include "zen/ui/layout-constants.h"
#include "zigzag/cairo-util.h"
#include "zigzag/layout.h"
#include "zigzag/node.h"

static void
zn_headset_dialog_item_headset_on_click(
    struct zigzag_node *node, double x, double y)
{
  UNUSED(node);
  UNUSED(x);
  UNUSED(y);
}

static bool
zn_headset_dialog_item_headset_render(struct zigzag_node *node, cairo_t *cr)
{
  struct zn_headset_dialog_item_headset *self = node->user_data;

  const double icon_width = 22.;
  const double icon_height = 12.5;
  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
  if (!zn_cairo_stamp_svg_on_surface(cr, VR_ICON_WHITE, 0,
          node->frame.height / 2 - icon_height / 2, icon_width, icon_height)) {
    return false;
  }

  char hmd[8];
  sprintf(hmd, "HMD %d", self->index);
  cairo_set_font_size(cr, 14);
  cairo_set_font_face(cr, zn_font_face_get_cairo_font_face(ZN_FONT_BOLD));
  zigzag_cairo_draw_text(cr, (char *)hmd, icon_width + 28.,
      node->frame.height / 2, ZIGZAG_ANCHOR_CENTER, ZIGZAG_ANCHOR_CENTER);
  cairo_set_font_face(cr, zn_font_face_get_cairo_font_face(ZN_FONT_REGULAR));

  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.5);
  cairo_set_font_size(cr, 12);
  zigzag_cairo_draw_text(cr, self->peer->wired ? "Wired" : self->peer->host,
      node->frame.width / 2, node->frame.height / 2, ZIGZAG_ANCHOR_CENTER,
      ZIGZAG_ANCHOR_CENTER);

  char *connection_status;
  if (self->peer->session) {
    connection_status = "Connected";
    cairo_set_source_rgba(cr, 0.06, 0.72, 0.13, 1.0);
  } else {
    connection_status = "Available";
    cairo_set_source_rgba(cr, 0.07, 0.51, 0.93, 1.0);
  }
  cairo_arc(cr, 230, node->frame.height / 2, 5, 0, 2 * M_PI);
  cairo_fill(cr);

  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
  cairo_set_font_size(cr, 13);
  zigzag_cairo_draw_text(cr, connection_status, 240., node->frame.height / 2.,
      ZIGZAG_ANCHOR_LEFT, ZIGZAG_ANCHOR_CENTER);
  return true;
}

static const struct zigzag_node_impl implementation = {
    .on_click = zn_headset_dialog_item_headset_on_click,
    .render = zn_headset_dialog_item_headset_render,
};

void
zn_headset_dialog_item_headset_handle_peer_new_session(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_headset_dialog_item_headset *self =
      zn_container_of(listener, self, peer_new_session_listener);
  struct zn_server *server = zn_server_get_singleton();
  zigzag_node_update_texture(self->zigzag_node, server->renderer);
}

struct zn_headset_dialog_item_headset *
zn_headset_dialog_item_headset_create(
    struct zigzag_layout *zigzag_layout, struct zn_peer *peer, uint32_t index)
{
  struct zn_headset_dialog_item_headset *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zigzag_node =
      zigzag_node_create(&implementation, zigzag_layout, true, self);
  if (self->zigzag_node == NULL) {
    zn_error("Failed to create a zigzag_node");
    goto err_free;
  }

  self->peer_new_session_listener.notify =
      zn_headset_dialog_item_headset_handle_peer_new_session;
  wl_signal_add(&peer->events.new_session, &self->peer_new_session_listener);

  self->zigzag_node->pending.frame.x =
      zigzag_layout->screen_width / 2. - vr_modal_headset_width / 2.;
  self->zigzag_node->pending.frame.width = vr_modal_headset_width;
  self->zigzag_node->pending.frame.height = 32.;

  self->peer = peer;
  self->index = index;

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_headset_dialog_item_headset_destroy(
    struct zn_headset_dialog_item_headset *self)
{
  wl_list_remove(&self->link);
  wl_list_remove(&self->peer_new_session_listener.link);
  zigzag_node_destroy(self->zigzag_node);
  free(self);
}
