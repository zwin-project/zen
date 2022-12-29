#include "zen/ui/nodes/power-button.h"

#include <cairo-ft.h>
#include <cairo.h>
#include <zen-common.h>
#include <zigzag.h>

#include "zen/server.h"

double icon_width = 10.;
double icon_height = 10.;

static int
zn_power_button_handle_second_timer(void *data)
{
  struct zn_power_button *self = (struct zn_power_button *)data;
  struct zigzag_node *zigzag_node = self->zigzag_node;
  struct zn_server *server = zn_server_get_singleton();

  int64_t time_ms = current_realtime_clock_ms();
  self->next_sec_ms = (time_ms - time_ms % MSEC_PER_SEC + MSEC_PER_SEC);

  int ms_delay = (int)(self->next_sec_ms - time_ms + 10);
  wl_event_source_timer_update(self->second_timer_source, ms_delay);

  zigzag_node_update_texture(zigzag_node, server->renderer);
  return 0;
}

static void
zn_power_button_on_click(struct zigzag_node *self, double x, double y)
{
  UNUSED(self);
  UNUSED(x);
  UNUSED(y);
  // TODO: Show "Logout"
}

static bool
zn_power_icon_render(struct zigzag_node *self, cairo_t *cr)
{
  UNUSED(self);
  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.7);
  cairo_paint(cr);
  bool result = zigzag_cairo_stamp_svg_on_surface(
      cr, POWER_BUTTON_ICON, 0., 0., icon_width, icon_height);
  if (!result) {
    return false;
  }
  return true;
}

static bool
zn_power_button_render(struct zigzag_node *self, cairo_t *cr)
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

  double padding = 6.;
  cairo_set_font_size(cr, 11);
  zigzag_cairo_draw_left_aligned_text(
      cr, output, self->frame->width, self->frame->height, padding);

  struct zn_power_button *power_button =
      (struct zn_power_button *)self->user_data;

  double icon_x = self->frame->width - padding - icon_width;
  double icon_y = (self->frame->height - icon_height) / 2;
  cairo_set_source_surface(
      cr, power_button->power_icon_surface, icon_x, icon_y);
  cairo_paint(cr);

  return true;
}

static void
zn_power_button_set_frame(
    struct zigzag_node *self, int output_width, int output_height)
{
  double margin_height = 6;
  double margin_width = 10;
  double button_width = 70.;
  double height_with_margin = 33.;
  double button_height = height_with_margin - margin_height * 2;

  self->frame->x = (double)output_width - button_width - margin_width;
  self->frame->y = (double)output_height - button_height - margin_height;
  self->frame->width = button_width;
  self->frame->height = button_height;
}

static const struct zigzag_node_impl implementation = {
    .on_click = zn_power_button_on_click,
    .set_frame = zn_power_button_set_frame,
    .render = zn_power_button_render,
};

struct zn_power_button *
zn_power_button_create(struct zigzag_layout *zigzag_layout,
    struct wlr_renderer *renderer, struct wl_display *display)
{
  struct zn_power_button *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  struct zigzag_node *zigzag_node =
      zigzag_node_create(&implementation, zigzag_layout, renderer, self);

  if (zigzag_node == NULL) {
    zn_error("Failed to create a zigzag_node");
    goto err_power_button;
  }
  self->zigzag_node = zigzag_node;

  self->power_icon_surface = zigzag_node_render_cairo_surface(
      zigzag_node, zn_power_icon_render, icon_width, icon_height);

  if (self->power_icon_surface == NULL) {
    zn_error("Failed to load the icon");
    goto err_zigzag_node;
  }

  self->second_timer_source =
      wl_event_loop_add_timer(wl_display_get_event_loop(display),
          zn_power_button_handle_second_timer, self);

  int64_t time_ms = current_realtime_clock_ms();
  self->next_sec_ms = (time_ms - time_ms % MSEC_PER_SEC + MSEC_PER_SEC);

  int ms_delay = (int)(self->next_sec_ms - time_ms + 10);
  wl_event_source_timer_update(self->second_timer_source, ms_delay);

  return self;

err_zigzag_node:
  zigzag_node_destroy(zigzag_node);

err_power_button:
  free(self);

err:
  return NULL;
}

void
zn_power_button_destroy(struct zn_power_button *self)
{
  wl_event_source_remove(self->second_timer_source);
  zigzag_node_destroy(self->zigzag_node);
  free(self);
}
