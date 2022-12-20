#include "zen/screen/zigzag-layout.h"

#include <cairo-ft.h>
#include <librsvg/rsvg.h>
#include <time.h>
#include <wlr/types/wlr_output_damage.h>
#include <zen-common.h>
#include <zigzag.h>

#include "zen/server.h"

const int menu_bar_height = 33.;

void
zen_zigzag_layout_on_damage(struct zigzag_node *node)
{
  zn_error("on_damage");
  zn_error("x: %d, y: %d, width: %d, height: %d", node->frame->x,
      node->frame->y, node->frame->width, node->frame->height);
  struct zen_zigzag_layout_state *state =
      (struct zen_zigzag_layout_state *)node->layout->state;
  wlr_output_damage_add_box(state->output->damage, node->frame);
}

void
power_button_on_click(struct zigzag_node *self, double x, double y)
{
  UNUSED(self);
  UNUSED(x);
  UNUSED(y);
  zn_warn("VR Mode starting...");
}

void
power_button_render(struct zigzag_node *self, cairo_t *cr)
{
  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.7);
  zigzag_cairo_draw_rounded_rectangle(
      cr, self->frame->width, self->frame->height, self->frame->height / 2);
  cairo_fill_preserve(cr);
  cairo_set_line_width(cr, 0.5);
  cairo_set_source_rgb(cr, 0.07, 0.12, 0.30);
  cairo_stroke(cr);

  FT_Library library;
  FT_Init_FreeType(&library);
  FT_Face face;
  FT_New_Face(
      library, "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf", 0, &face);

  cairo_font_face_t *cr_face = cairo_ft_font_face_create_for_ft_face(face, 0);
  cairo_set_font_face(cr, cr_face);

  time_t rawtime;
  struct tm *timeinfo;

  char output[8];

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  sprintf(output, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
  zn_error("Time: %s", output);

  double padding = 6.;
  cairo_set_font_size(cr, 11);
  zigzag_cairo_draw_left_aligned_text(
      cr, output, self->frame->width, self->frame->height, padding);

  GError *error = NULL;
  GFile *file = g_file_new_for_path(POWER_BUTTON_ICON);
  RsvgHandle *handle = rsvg_handle_new_from_gfile_sync(
      file, RSVG_HANDLE_FLAGS_NONE, NULL, &error);
  if (handle == NULL) {
    zn_error("Failed to create the svg handler: %s", error->message);
    goto err;
  }
  double icon_width = 10.;
  double icon_height = 10.;
  RsvgRectangle viewport = {
      .x = self->frame->width - padding - icon_width,
      .y = (self->frame->height - icon_height) / 2,
      .width = icon_width,
      .height = icon_height,
  };
  if (!rsvg_handle_render_document(handle, cr, &viewport, &error)) {
    zn_error("Failed to render the svg");
  }

  g_object_unref(handle);

err:
  return;
}

void
power_button_set_frame(
    struct zigzag_node *self, int output_width, int output_height)
{
  double margin_height = 6;
  double margin_width = 10;
  double button_width = 70.;
  double button_height = menu_bar_height - margin_height * 2;

  zn_error("output: %d", output_width);

  self->frame->x = (double)output_width - button_width - margin_width;
  self->frame->y = (double)output_height - button_height - margin_height;
  self->frame->width = button_width;
  self->frame->height = button_height;
}

struct zigzag_node *
create_power_button(struct zigzag_layout *node_layout, struct zn_server *server)
{
  struct zigzag_node *power_button =
      zigzag_node_create(node_layout, NULL, server->renderer,
          power_button_set_frame, power_button_on_click, power_button_render);

  if (power_button == NULL) {
    zn_error("Failed to create the VR button");
    goto err;
  }
  return power_button;
err:
  return NULL;
}

void
menu_bar_on_click(struct zigzag_node *self, double x, double y)
{
  UNUSED(self);
  UNUSED(x);
  UNUSED(y);
}

void
menu_bar_render(struct zigzag_node *self, cairo_t *cr)
{
  UNUSED(self);
  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.1);
  cairo_paint(cr);
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.88);
  cairo_set_line_width(cr, 0.25);
  cairo_rectangle(cr, 0., 0., self->frame->width, self->frame->height);
  cairo_stroke(cr);
}

void
menu_bar_set_frame(
    struct zigzag_node *self, int output_width, int output_height)
{
  double bar_width = (double)output_width;

  self->frame->x = 0.;
  self->frame->y = (double)output_height - menu_bar_height;
  self->frame->width = bar_width;
  self->frame->height = menu_bar_height;
}

struct zigzag_node *
create_menu_bar(struct zigzag_layout *node_layout, struct zn_server *server,
    struct zigzag_node *power_button)
{
  struct zigzag_node *menu_bar = zigzag_node_create(node_layout, NULL,
      server->renderer, menu_bar_set_frame, menu_bar_on_click, menu_bar_render);

  if (menu_bar == NULL) {
    zn_error("Failed to create the menu bar");
    goto err;
  }
  wl_list_insert(&menu_bar->children, &power_button->link);
  return menu_bar;
err:
  return NULL;
}

void
zen_zigzag_layout_setup_default(
    struct zigzag_layout *node_layout, struct zn_server *server)
{
  struct zigzag_node *power_button = create_power_button(node_layout, server);
  struct zen_zigzag_layout_state *state =
      (struct zen_zigzag_layout_state *)node_layout->state;
  state->output->power_button = power_button;
  struct zigzag_node *menu_bar =
      create_menu_bar(node_layout, server, power_button);
  wl_list_insert(&node_layout->nodes, &menu_bar->link);
}
