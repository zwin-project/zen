#include "zen/ui/nodes/vr-menu/vr-menu.h"

#include <cairo.h>
#include <zen-common.h>
#include <zigzag.h>

#include "zen/peer.h"
#include "zen/server.h"
#include "zen/ui/layout-constants.h"
#include "zen/ui/nodes/vr-menu/connect-button.h"
#include "zen/ui/nodes/vr-menu/headset.h"

static void
zn_vr_menu_refresh_headsets(struct zn_vr_menu *self, struct wl_list *peer_list)
{
  struct zn_vr_menu_item_headset *headset;
  wl_list_for_each (headset, &self->headset_list, link) {
    zn_vr_menu_item_headset_destroy(headset);
  }
  struct zn_peer *peer_iter;
  struct zn_server *server = zn_server_get_singleton();
  int i = 0;
  wl_list_for_each (peer_iter, peer_list, link) {
    struct zn_vr_menu_item_headset *headset = zn_vr_menu_item_headset_create(
        self->zigzag_node->layout, server->renderer, peer_iter, i);
    wl_list_insert(&self->zigzag_node->node_list, &headset->zigzag_node->link);
    wl_list_insert(&self->headset_list, &headset->link);
    ++i;
  }
}

static void
zn_vr_menu_handle_peer_list_changed(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_vr_menu *self =
      zn_container_of(listener, self, peer_list_changed_listener);
  struct zn_server *server = zn_server_get_singleton();
  struct zn_remote *remote = server->remote;
  zn_vr_menu_refresh_headsets(self, &remote->peer_list);

  zigzag_node_update_frame(self->zigzag_node);
  zigzag_node_update_texture(self->zigzag_node, server->renderer);
}

static void
zn_vr_menu_on_click(struct zigzag_node *self, double x, double y)
{
  UNUSED(self);
  UNUSED(x);
  UNUSED(y);
}

static bool
zn_vr_menu_render(struct zigzag_node *self, cairo_t *cr)
{
  struct zn_vr_menu *vr_menu = (struct zn_vr_menu *)self->user_data;
  cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  zigzag_cairo_draw_rounded_bubble(cr, 0., 0., self->frame.width,
      self->frame.height, 5., vr_menu->tip_x - self->frame.x);
  cairo_fill_preserve(cr);
  cairo_set_line_width(cr, 0.5);
  cairo_set_source_rgba(cr, 0., 0., 0., 0.12);
  cairo_stroke(cr);

  double padding_height = vr_menu_bubble_padding_height;
  double padding_width = (vr_menu_bubble_width - vr_menu_headset_width) / 2;

  struct zn_server *server = zn_server_get_singleton();
  struct zn_remote *remote = server->remote;
  cairo_set_font_size(cr, 13);
  double content_height;
  if (wl_list_empty(&remote->peer_list)) {
    cairo_set_source_rgb(cr, 0., 0., 0.);
    zigzag_cairo_draw_text(cr, "No headsets found", vr_menu_bubble_width / 2,
        vr_menu_bubble_padding_height + vr_headsets_heading_height +
            no_headsets_text_height / 2 - 5.,
        ZIGZAG_ANCHOR_CENTER, ZIGZAG_ANCHOR_CENTER);
    content_height = no_headsets_text_height;
  } else {
    content_height =
        wl_list_length(&remote->peer_list) * vr_menu_headset_height;
  }

  cairo_set_source_rgb(cr, .5, .5, .5);
  zigzag_cairo_draw_text(cr, "Headsets", padding_width + 15,
      vr_menu_bubble_padding_height + vr_headsets_heading_height / 2,
      ZIGZAG_ANCHOR_LEFT, ZIGZAG_ANCHOR_CENTER);

  zigzag_cairo_draw_rounded_rectangle(cr, padding_width, padding_height,
      vr_menu_headset_width, vr_headsets_heading_height + content_height + 5.,
      5.);
  cairo_set_source_rgba(cr, 0., 0., 0., 0.5);
  cairo_stroke(cr);

  cairo_set_source_rgb(cr, .35, .39, .51);
  zigzag_cairo_draw_text(cr, "How can I connect my headset?", padding_width,
      vr_menu_bubble_padding_height + vr_headsets_heading_height +
          content_height + vr_how_to_connect_height / 2,
      ZIGZAG_ANCHOR_LEFT, ZIGZAG_ANCHOR_CENTER);

  return true;
}

static void
zn_vr_menu_set_frame(
    struct zigzag_node *self, double screen_width, double screen_height)
{
  self->frame.x = screen_width - vr_menu_bubble_width - vr_menu_space_right;
  self->frame.width = vr_menu_bubble_width;

  struct zn_server *server = zn_server_get_singleton();
  struct zn_remote *remote = server->remote;
  if (wl_list_empty(&remote->peer_list)) {
    self->frame.height = tip_height + vr_how_to_connect_height +
                         no_headsets_text_height + vr_headsets_heading_height +
                         vr_menu_bubble_padding_height;
  } else {
    self->frame.height =
        tip_height + vr_how_to_connect_height +
        wl_list_length(&remote->peer_list) * vr_menu_headset_height +
        vr_headsets_heading_height + vr_menu_bubble_padding_height;
  }
  self->frame.y = screen_height - self->frame.height - menu_bar_height;
}

static const struct zigzag_node_impl implementation = {
    .on_click = zn_vr_menu_on_click,
    .set_frame = zn_vr_menu_set_frame,
    .render = zn_vr_menu_render,
};

struct zn_vr_menu *
zn_vr_menu_create(struct zigzag_layout *zigzag_layout,
    struct wlr_renderer *renderer, double tip_x)
{
  struct zn_vr_menu *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }
  self->tip_x = tip_x;

  struct zigzag_node *zigzag_node =
      zigzag_node_create(&implementation, zigzag_layout, renderer, false, self);

  if (zigzag_node == NULL) {
    zn_error("Failed to create a zigzag_node");
    goto err_vr_menu;
  }
  self->zigzag_node = zigzag_node;

  struct zn_server *server = zn_server_get_singleton();
  self->peer_list_changed_listener.notify = zn_vr_menu_handle_peer_list_changed;
  wl_signal_add(&server->remote->events.peer_list_changed,
      &self->peer_list_changed_listener);

  wl_list_init(&self->headset_list);

  return self;

err_vr_menu:
  free(self);

err:
  return NULL;
}

void
zn_vr_menu_destroy(struct zn_vr_menu *self)
{
  struct zn_vr_menu_item_headset *headset;
  wl_list_for_each (headset, &self->headset_list, link) {
    zn_vr_menu_item_headset_destroy(headset);
  }
  wl_list_remove(&self->headset_list);
  zigzag_node_destroy(self->zigzag_node);
  free(self);
}
