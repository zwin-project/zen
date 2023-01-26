#include "zen/ui/nodes/vr-modal/headset-dialog.h"

#include <cairo.h>
#include <math.h>
#include <string.h>
#include <zen-common.h>
#include <znr-remote.h>

#include "zen/peer.h"
#include "zen/screen/output.h"
#include "zen/server.h"
#include "zen/ui/layout-constants.h"
#include "zen/ui/nodes/textbox.h"
#include "zen/ui/nodes/vr-modal/headset.h"
#include "zigzag/cairo-util.h"
#include "zigzag/layout.h"
#include "zigzag/node.h"

#define ICON_WIDTH 140.0
#define ICON_HEIGHT 80.0

static void
zn_vr_modal_item_headset_dialog_update(
    struct zn_vr_modal_item_headset_dialog *self)
{
  struct zn_server *server = zn_server_get_singleton();

  struct zn_peer *peer;
  int i = 1;
  wl_list_for_each (peer, &server->remote->peer_list, link) {
    struct zn_headset_dialog_item_headset *headset =
        zn_headset_dialog_item_headset_create(
            self->zigzag_node->layout, peer, i);
    if (!headset) {
      zn_error("Failed to create zn_headset_dialog_item_headset");
      continue;
    }
    wl_list_insert(&self->headset_list, &headset->link);
    zigzag_node_add_child(
        self->zigzag_node, headset->zigzag_node, server->renderer);
    ++i;
  }

  if (wl_list_empty(&self->headset_list)) {
    self->no_headset = zn_textbox_create(self->zigzag_node->layout,
        "No headset detected", 14, (struct zigzag_color){1., 1., 1., 1.},
        (struct zigzag_color){0., 0., 0., 0.}, 0);
    if (!self->no_headset) {
      zn_error("Failed to create zn_headset_dialog_item_headset");
    } else {
      zigzag_node_add_child(
          self->zigzag_node, self->no_headset->zigzag_node, server->renderer);
    }
  }

  double width, height;
  zigzag_node_child_total_size(self->zigzag_node, &width, &height);
  self->zigzag_node->pending.frame.height = height + 40;
  zigzag_node_update_frame(self->zigzag_node);
  zigzag_node_reconfigure(
      self->zigzag_node, ZIGZAG_RECONFIGURE_VERTICAL, ZIGZAG_RECONFIGURE_START);
  zigzag_node_update_texture(self->zigzag_node, server->renderer);
}

static void
zn_vr_modal_item_headset_dialog_destroy_children(
    struct zn_vr_modal_item_headset_dialog *self)
{
  struct zn_headset_dialog_item_headset *headset, *tmp;
  wl_list_for_each_safe (headset, tmp, &self->headset_list, link) {
    zn_headset_dialog_item_headset_destroy(headset);
  }

  if (self->no_headset) {
    zn_textbox_destroy(self->no_headset);
    self->no_headset = NULL;
  }
}

static void
zn_vr_modal_item_headset_dialog_handle_peer_list_changed(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_vr_modal_item_headset_dialog *self =
      zn_container_of(listener, self, peer_list_changed_listener);

  zn_vr_modal_item_headset_dialog_destroy_children(self);
  zn_vr_modal_item_headset_dialog_update(self);
}

static void
zn_vr_modal_item_headset_dialog_on_click(
    struct zigzag_node *node, double x, double y)
{
  UNUSED(node);
  UNUSED(x);
  UNUSED(y);
}

static bool
zn_vr_modal_item_headset_dialog_render(struct zigzag_node *node, cairo_t *cr)
{
  zn_cairo_draw_rounded_rectangle(
      cr, 0, 0, node->frame.width, node->frame.height, 8.0);

  zigzag_cairo_draw_node_frame(cr, node,
      (struct zigzag_color){0.0, 0.0, 0.0, 0.0},
      (struct zigzag_color){1.0, 1.0, 1.0, 1.0}, 2.0, 8.0);

  return true;
}

static const struct zigzag_node_impl implementation = {
    .on_click = zn_vr_modal_item_headset_dialog_on_click,
    .render = zn_vr_modal_item_headset_dialog_render,
};

struct zn_vr_modal_item_headset_dialog *
zn_vr_modal_item_headset_dialog_create(
    struct zigzag_layout *zigzag_layout, struct wlr_renderer *renderer)
{
  struct zn_vr_modal_item_headset_dialog *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  struct zigzag_node *zigzag_node =
      zigzag_node_create(&implementation, zigzag_layout, true, self);

  if (zigzag_node == NULL) {
    zn_error("Failed to create a zigzag_node");
    goto err_headset_dialog;
  }
  self->zigzag_node = zigzag_node;

  self->headset_text = zn_textbox_create(zigzag_layout, "Headset", 13,
      (struct zigzag_color){1., 1., 1., 0.5},
      (struct zigzag_color){0., 0., 0., 0.}, 0);
  if (!self->headset_text) {
    zn_error("Failed to create a zn_textbox");
    goto err_node;
  }
  self->headset_text->zigzag_node->pending.frame.x =
      zigzag_layout->screen_width / 2 -
      self->headset_text->zigzag_node->pending.frame.width / 2;
  zigzag_node_add_child(
      self->zigzag_node, self->headset_text->zigzag_node, renderer);

  wl_list_init(&self->headset_list);

  struct zn_server *server = zn_server_get_singleton();
  self->peer_list_changed_listener.notify =
      zn_vr_modal_item_headset_dialog_handle_peer_list_changed;
  wl_signal_add(&server->remote->events.peer_list_changed,
      &self->peer_list_changed_listener);

  self->zigzag_node->padding.top = self->zigzag_node->padding.bottom = 16.0;
  self->zigzag_node->pending.frame.x =
      zigzag_layout->screen_width / 2 - vr_modal_headset_dialog_width / 2;
  self->zigzag_node->pending.frame.y = zigzag_layout->screen_height / 2 + 60;
  self->zigzag_node->pending.frame.width = vr_modal_headset_dialog_width;

  zn_vr_modal_item_headset_dialog_update(self);

  return self;

err_node:
  zigzag_node_destroy(self->zigzag_node);

err_headset_dialog:
  free(self);

err:
  return NULL;
}

void
zn_vr_modal_item_headset_dialog_destroy(
    struct zn_vr_modal_item_headset_dialog *self)
{
  wl_list_remove(&self->peer_list_changed_listener.link);
  zn_vr_modal_item_headset_dialog_destroy_children(self);
  zn_textbox_destroy(self->headset_text);
  zigzag_node_destroy(self->zigzag_node);
  free(self);
}
