#include "zen/ui/nodes/vr-modal.h"

#include <cairo.h>
#include <string.h>
#include <zen-common.h>
#include <zigzag.h>
#include <znr-remote.h>

#include "zen/peer.h"
#include "zen/screen/output.h"
#include "zen/server.h"

#define BG_ALPHA 0.7
#define HEADSET_DIALOG_WIDTH 400
#define ICON_WIDTH 140.0
#define ICON_HEIGHT 80.0

static void
zn_vr_modal_handle_peer_list_changed(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_vr_modal *self =
      zn_container_of(listener, self, peer_list_changed_listener);
  struct zn_server *server = zn_server_get_singleton();
  zigzag_node_update_texture(self->zigzag_node, server->renderer);
}

static void
zn_vr_modal_on_click(struct zigzag_node *node, double x, double y)
{
  UNUSED(node);
  UNUSED(x);
  UNUSED(y);
}

static void
zn_vr_modal_render_headset_status(cairo_t *cr, struct zn_peer *peer,
    double center_x, double center_y, int index)
{
  cairo_save(cr);
  cairo_set_font_size(cr, 12);

  double w = ICON_WIDTH * 0.18;
  double h = ICON_HEIGHT * 0.18;
  if (!zigzag_cairo_stamp_svg_on_surface(
          cr, VR_ICON, (center_x - w / 2) - 120, center_y - h / 2, w, h)) {
    return;
  }

  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
  char hmd[8];
  sprintf(hmd, "HMD %d", index);
  zigzag_cairo_draw_text(cr, (char *)hmd, center_x - 80, center_y,
      ZIGZAG_ANCHOR_CENTER, ZIGZAG_ANCHOR_CENTER);

  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.5);
  zigzag_cairo_draw_text(cr, peer->host, center_x, center_y,
      ZIGZAG_ANCHOR_CENTER, ZIGZAG_ANCHOR_CENTER);

  char *connection_status;
  if (peer->session) {
    connection_status = "Connected";
    cairo_set_source_rgba(cr, 0.06, 0.72, 0.13, 1.0);
  } else {
    connection_status = "Available";
    cairo_set_source_rgba(cr, 0.07, 0.51, 0.93, 1.0);
  }
  cairo_arc(cr, center_x + 70, center_y, 5, 0, 2 * M_PI);
  cairo_fill(cr);

  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
  zigzag_cairo_draw_text(cr, connection_status, center_x + 80, center_y,
      ZIGZAG_ANCHOR_LEFT, ZIGZAG_ANCHOR_CENTER);

  cairo_restore(cr);
}

static void
zn_vr_modal_render_headset_dialog(struct zn_vr_modal *self, cairo_t *cr,
    double center_top_x, double center_top_y)
{
  UNUSED(self);
  struct zn_server *server = zn_server_get_singleton();
  struct zn_remote *remote = server->remote;
  const int peer_list_length = wl_list_length(&remote->peer_list);
  const double headset_dialog_height =
      60 + 30 * (peer_list_length - 1 > 0 ? peer_list_length - 1 : 0);

  cairo_save(cr);

  zigzag_cairo_draw_rounded_rectangle(cr,
      center_top_x - HEADSET_DIALOG_WIDTH / 2, center_top_y,
      HEADSET_DIALOG_WIDTH, headset_dialog_height, 8.0);
  cairo_stroke_preserve(cr);
  cairo_set_line_width(cr, 1.0);
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
  cairo_fill(cr);

  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.5);
  cairo_set_font_size(cr, 13);
  zigzag_cairo_draw_text(cr, "Headset", center_top_x, center_top_y + 10,
      ZIGZAG_ANCHOR_CENTER, ZIGZAG_ANCHOR_TOP);

  if (wl_list_empty(&remote->peer_list)) {
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    cairo_set_font_size(cr, 14);
    zigzag_cairo_draw_text(cr, "No headset detected", center_top_x,
        center_top_y + 40, ZIGZAG_ANCHOR_CENTER, ZIGZAG_ANCHOR_CENTER);
  } else {
    int i = 0;
    struct zn_peer *peer_iter;
    wl_list_for_each (peer_iter, &remote->peer_list, link) {
      zn_vr_modal_render_headset_status(
          cr, peer_iter, center_top_x, center_top_y + 40 * (i + 1), i);
      ++i;
    }
  }

  cairo_restore(cr);
}

static void
zn_vr_modal_render_exit_key_description(
    cairo_t *cr, double center_x, double center_y)
{
  cairo_save(cr);
  cairo_set_font_size(cr, 14);

  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
  zigzag_cairo_draw_text(cr, "Press    Meta    +    V    to exit VR mode",
      center_x, center_y, ZIGZAG_ANCHOR_CENTER, ZIGZAG_ANCHOR_CENTER);

  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.3);
  zigzag_cairo_draw_rounded_rectangle(
      cr, center_x - 77, center_y - 8, 44, 20, 4);
  cairo_fill(cr);
  zigzag_cairo_draw_rounded_rectangle(
      cr, center_x - 14, center_y - 8, 24, 20, 4);
  cairo_fill(cr);

  cairo_restore(cr);
}

static bool
zn_vr_modal_render(struct zigzag_node *node, cairo_t *cr)
{
  struct zn_vr_modal *self = node->user_data;
  double center_x = node->frame.width / 2;
  double center_y = node->frame.height / 2;

  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, BG_ALPHA);
  cairo_paint(cr);

  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);

  if (!zigzag_cairo_stamp_svg_on_surface(cr, VR_ICON, center_x - ICON_WIDTH / 2,
          (center_y - ICON_HEIGHT / 2) - 120, ICON_WIDTH, ICON_HEIGHT)) {
    return false;
  }

  cairo_set_font_size(cr, 32);
  zigzag_cairo_draw_text(cr, "You're in VR", center_x, center_y - 30,
      ZIGZAG_ANCHOR_CENTER, ZIGZAG_ANCHOR_CENTER);
  cairo_set_font_size(cr, 14);
  zigzag_cairo_draw_text(cr, "Put on your headset to start working in VR",
      center_x, center_y, ZIGZAG_ANCHOR_CENTER, ZIGZAG_ANCHOR_CENTER);

  zn_vr_modal_render_headset_dialog(self, cr, center_x, center_y + 60);

  zn_vr_modal_render_exit_key_description(
      cr, center_x, node->frame.height - 60);

  return true;
}

static void
zn_vr_modal_set_frame(
    struct zigzag_node *node, double screen_width, double screen_height)
{
  node->frame.x = 0.;
  node->frame.y = 0.;
  node->frame.width = screen_width;
  node->frame.height = screen_height;
}

static const struct zigzag_node_impl implementation = {
    .on_click = zn_vr_modal_on_click,
    .set_frame = zn_vr_modal_set_frame,
    .render = zn_vr_modal_render,
};

struct zn_vr_modal *
zn_vr_modal_create(
    struct zigzag_layout *zigzag_layout, struct wlr_renderer *renderer)
{
  struct zn_vr_modal *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  struct zigzag_node *zigzag_node =
      zigzag_node_create(&implementation, zigzag_layout, renderer, false, self);

  if (zigzag_node == NULL) {
    zn_error("Failed to create a zigzag_node");
    goto err_vr_modal;
  }
  self->zigzag_node = zigzag_node;

  struct zn_server *server = zn_server_get_singleton();
  self->peer_list_changed_listener.notify =
      zn_vr_modal_handle_peer_list_changed;
  wl_signal_add(&server->remote->events.peer_list_changed,
      &self->peer_list_changed_listener);

  return self;

err_vr_modal:
  free(self);

err:
  return NULL;
}

void
zn_vr_modal_destroy(struct zn_vr_modal *self)
{
  wl_list_remove(&self->peer_list_changed_listener.link);
  zigzag_node_destroy(self->zigzag_node);
  free(self);
}
