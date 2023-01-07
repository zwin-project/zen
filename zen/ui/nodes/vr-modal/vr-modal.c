#include "zen/ui/nodes/vr-modal/vr-modal.h"

#include <cairo.h>
#include <string.h>
#include <zen-common.h>
#include <zigzag.h>
#include <znr-remote.h>

#include "zen/screen/output.h"
#include "zen/server.h"
#include "zen/ui/nodes/vr-modal/headset-dialog.h"

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

  self->headset_dialog = zn_headset_dialog_create(zigzag_layout, renderer);
  if (self->headset_dialog == NULL) {
    zn_error("Failed to create zn_headset_dialog");
    goto err_zigzag_node;
  }
  wl_list_insert(
      &self->zigzag_node->node_list, &self->headset_dialog->zigzag_node->link);

  return self;

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
  zn_headset_dialog_destroy(self->headset_dialog);
  zigzag_node_destroy(self->zigzag_node);
  free(self);
}
