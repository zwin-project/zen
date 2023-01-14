#include "zen/ui/nodes/vr-modal/keybind-description.h"

#include <cairo.h>
#include <math.h>
#include <string.h>
#include <zen-common.h>
#include <zigzag.h>
#include <znr-remote.h>

#include "zen/peer.h"
#include "zen/screen/output.h"
#include "zen/server.h"
#include "zen/ui/layout-constants.h"
#include "zen/ui/nodes/vr-modal/textbox.h"

static void
zn_vr_modal_item_keybind_description_on_click(
    struct zigzag_node *node, double x, double y)
{
  UNUSED(node);
  UNUSED(x);
  UNUSED(y);
}

static bool
zn_vr_modal_item_keybind_description_render(
    struct zigzag_node *node, cairo_t *cr)
{
  UNUSED(node);
  UNUSED(cr);
  return true;
}

static const struct zigzag_node_impl implementation = {
    .on_click = zn_vr_modal_item_keybind_description_on_click,
    .render = zn_vr_modal_item_keybind_description_render,
};

struct zn_vr_modal_item_keybind_description *
zn_vr_modal_item_keybind_description_create(
    struct zigzag_layout *zigzag_layout, struct wlr_renderer *renderer)
{
  struct zn_vr_modal_item_keybind_description *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  struct zigzag_node *zigzag_node =
      zigzag_node_create(&implementation, zigzag_layout, true, self);

  if (zigzag_node == NULL) {
    zn_error("Failed to create a zigzag_node");
    goto err_free;
  }
  self->zigzag_node = zigzag_node;

  const double y = zigzag_layout->screen_height - 70.;

  self->press = zn_textbox_create(zigzag_layout, "Press", 14,
      (struct zigzag_color){1., 1., 1., 1.},
      (struct zigzag_color){0., 0., 0., 0.}, 0);
  if (!self->press) {
    zn_error("Failed to create zn_textbox");
    goto err_zigzag_node;
  }
  self->press->zigzag_node->pending.frame.y = y;
  zigzag_node_add_child(self->zigzag_node, self->press->zigzag_node, renderer);

  self->meta_key = zn_textbox_create(zigzag_layout, "Meta", 14,
      (struct zigzag_color){1., 1., 1., 1.},
      (struct zigzag_color){1., 1., 1., 0.3}, 4.);
  if (!self->meta_key) {
    zn_error("Failed to create zn_textbox");
    goto err_press;
  }
  self->meta_key->zigzag_node->pending.frame.y = y;
  zigzag_node_add_child(
      self->zigzag_node, self->meta_key->zigzag_node, renderer);

  self->plus = zn_textbox_create(zigzag_layout, "+", 14,
      (struct zigzag_color){1., 1., 1., 1.},
      (struct zigzag_color){0., 0., 0., 0.}, 0);
  if (!self->plus) {
    zn_error("Failed to create zn_textbox");
    goto err_meta_key;
  }
  self->plus->zigzag_node->pending.frame.y = y;
  zigzag_node_add_child(self->zigzag_node, self->plus->zigzag_node, renderer);

  self->v_key = zn_textbox_create(zigzag_layout, "V", 14,
      (struct zigzag_color){1., 1., 1., 1.},
      (struct zigzag_color){1., 1., 1., 0.3}, 4.);
  if (!self->v_key) {
    zn_error("Failed to create zn_textbox");
    goto err_plus;
  }
  self->v_key->zigzag_node->pending.frame.y = y;
  zigzag_node_add_child(self->zigzag_node, self->v_key->zigzag_node, renderer);

  self->to_exit = zn_textbox_create(zigzag_layout, "to exit VR mode", 14,
      (struct zigzag_color){1., 1., 1., 1.},
      (struct zigzag_color){0., 0., 0., 0.}, 0);
  if (!self->to_exit) {
    zn_error("Failed to create zn_textbox");
    goto err_v_key;
  }
  self->to_exit->zigzag_node->pending.frame.y = y;
  zigzag_node_add_child(
      self->zigzag_node, self->to_exit->zigzag_node, renderer);

  self->zigzag_node->margin.left = 64.;
  self->zigzag_node->pending.frame.x =
      zigzag_layout->screen_width / 2 - vr_modal_keybind_description_width / 2;
  self->zigzag_node->pending.frame.y = y;
  self->zigzag_node->pending.frame.width = vr_modal_keybind_description_width;
  self->zigzag_node->pending.frame.height = vr_modal_keybind_description_height;
  zigzag_node_update_frame(self->zigzag_node);
  zigzag_node_reconfigure(self->zigzag_node, ZIGZAG_RECONFIGURE_HORIZONTAL,
      ZIGZAG_RECONFIGURE_JUSTIFY);

  return self;

err_v_key:
  zn_textbox_destroy(self->v_key);

err_plus:
  zn_textbox_destroy(self->plus);

err_meta_key:
  zn_textbox_destroy(self->meta_key);

err_press:
  zn_textbox_destroy(self->press);

err_zigzag_node:
  zigzag_node_destroy(self->zigzag_node);

err_free:
  free(self);

err:
  return NULL;
}

void
zn_vr_modal_item_keybind_description_destroy(
    struct zn_vr_modal_item_keybind_description *self)
{
  zn_textbox_destroy(self->press);
  zn_textbox_destroy(self->meta_key);
  zn_textbox_destroy(self->plus);
  zn_textbox_destroy(self->v_key);
  zn_textbox_destroy(self->to_exit);

  zigzag_node_destroy(self->zigzag_node);
  free(self);
}
