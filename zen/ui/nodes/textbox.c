#include "zen/ui/nodes/textbox.h"

#include <cairo.h>
#include <string.h>
#include <zen-common.h>
#include <zigzag.h>

#include "zen/screen/output.h"
#include "zen/ui/layout-constants.h"

static void
zn_textbox_on_click(struct zigzag_node *node, double x, double y)
{
  UNUSED(node);
  UNUSED(x);
  UNUSED(y);
}

static bool
zn_textbox_render(struct zigzag_node *node, cairo_t *cr)
{
  struct zn_textbox *self = node->user_data;
  zigzag_cairo_draw_node_frame(cr, node, self->bg_color,
      (struct zigzag_color){0., 0., 0., 0.}, 0., self->frame_radius);

  cairo_set_font_size(cr, self->font_size);
  cairo_set_source_rgba(cr, self->font_color.r, self->font_color.g,
      self->font_color.b, self->font_color.a);
  zigzag_cairo_draw_text(cr, self->text, node->frame.width / 2,
      node->frame.height / 2, ZIGZAG_ANCHOR_CENTER, ZIGZAG_ANCHOR_CENTER);

  return true;
}

static const struct zigzag_node_impl implementation = {
    .on_click = zn_textbox_on_click,
    .render = zn_textbox_render,
};

struct zn_textbox *
zn_textbox_create(struct zigzag_layout *zigzag_layout, char *text,
    double font_size, struct zigzag_color font_color,
    struct zigzag_color bg_color, double frame_radius)
{
  struct zn_textbox *self;

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

  self->text = strdup(text);
  self->font_size = font_size;
  self->font_color = font_color;
  self->bg_color = bg_color;
  self->frame_radius = frame_radius;

  cairo_text_extents_t extents;
  zn_cairo_get_text_extents(self->text, self->font_size,
      zn_font_face_get_cairo_font_face(ZN_FONT_REGULAR), &extents);
  self->zigzag_node->pending.frame.width =
      extents.width - extents.x_bearing + 20.;
  self->zigzag_node->pending.frame.height = vr_modal_keybind_description_height;

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_textbox_destroy(struct zn_textbox *self)
{
  free(self->text);
  zigzag_node_destroy(self->zigzag_node);
  free(self);
}
