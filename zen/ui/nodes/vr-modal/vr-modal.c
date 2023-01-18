#include "zen/ui/nodes/vr-modal/vr-modal.h"

#include <cairo.h>
#include <string.h>
#include <zen-common.h>
#include <znr-remote.h>

#include "zen/screen/output.h"
#include "zen/server.h"
#include "zen/ui/nodes/vr-modal/headset-dialog.h"
#include "zen/ui/nodes/vr-modal/keybind-description.h"
#include "zigzag/cairo-util.h"
#include "zigzag/layout.h"
#include "zigzag/node.h"

#define BG_ALPHA 0.7
#define ICON_WIDTH 140.0
#define ICON_HEIGHT 80.0

static void
zn_vr_modal_on_click(struct zigzag_node *node, double x, double y)
{
  UNUSED(node);
  UNUSED(x);
  UNUSED(y);
}

static bool
zn_vr_modal_render(struct zigzag_node *node, cairo_t *cr)
{
  double center_x = node->frame.width / 2;
  double center_y = node->frame.height / 2;

  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, BG_ALPHA);
  cairo_paint(cr);

  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);

  if (!zn_cairo_stamp_svg_on_surface(cr, VR_ICON_WHITE,
          center_x - ICON_WIDTH / 2, (center_y - ICON_HEIGHT / 2) - 120,
          ICON_WIDTH, ICON_HEIGHT)) {
    return false;
  }

  cairo_set_font_size(cr, 32);
  zigzag_cairo_draw_text(cr, "You're in VR", center_x, center_y - 30,
      ZIGZAG_ANCHOR_CENTER, ZIGZAG_ANCHOR_CENTER);
  cairo_set_font_size(cr, 14);
  zigzag_cairo_draw_text(cr, "Put on your headset to start working in VR",
      center_x, center_y, ZIGZAG_ANCHOR_CENTER, ZIGZAG_ANCHOR_CENTER);

  return true;
}

static const struct zigzag_node_impl implementation = {
    .on_click = zn_vr_modal_on_click,
    .render = zn_vr_modal_render,
};

static void
zn_vr_modal_init_frame(
    struct zigzag_node *node, double screen_width, double screen_height)
{
  node->pending.frame.x = 0.;
  node->pending.frame.y = 0.;
  node->pending.frame.width = screen_width;
  node->pending.frame.height = screen_height;
}

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
      zigzag_node_create(&implementation, zigzag_layout, false, self);

  if (zigzag_node == NULL) {
    zn_error("Failed to create a zigzag_node");
    goto err_vr_modal;
  }
  self->zigzag_node = zigzag_node;

  self->headset_dialog =
      zn_vr_modal_item_headset_dialog_create(zigzag_layout, renderer);
  if (self->headset_dialog == NULL) {
    zn_error("Failed to create zn_vr_modal_item_headset_dialog");
    goto err_zigzag_node;
  }
  zigzag_node_add_child(
      self->zigzag_node, self->headset_dialog->zigzag_node, renderer);

  self->keybind_description =
      zn_vr_modal_item_keybind_description_create(zigzag_layout, renderer);
  if (!self->keybind_description) {
    zn_error("Failed to create zn_vr_modal_item_keybind_description");
    goto err_headset_dialog;
  }
  zigzag_node_add_child(
      self->zigzag_node, self->keybind_description->zigzag_node, renderer);

  zn_vr_modal_init_frame(self->zigzag_node,
      self->zigzag_node->layout->screen_width,
      self->zigzag_node->layout->screen_height);

  return self;

err_headset_dialog:
  zn_vr_modal_item_headset_dialog_destroy(self->headset_dialog);

err_zigzag_node:
  zigzag_node_destroy(self->zigzag_node);

err_vr_modal:
  free(self);

err:
  return NULL;
}

void
zn_vr_modal_destroy(struct zn_vr_modal *self)
{
  zn_vr_modal_item_headset_dialog_destroy(self->headset_dialog);
  zigzag_node_destroy(self->zigzag_node);
  free(self);
}
