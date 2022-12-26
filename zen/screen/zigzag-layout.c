#include "zen/screen/zigzag-layout.h"

#include <cairo-ft.h>
#include <librsvg/rsvg.h>
#include <time.h>
#include <wlr/types/wlr_output_damage.h>
#include <zen-common.h>
#include <zigzag.h>

#include "zen/server.h"

const int menu_bar_height = 33;

struct zn_zigzag_layout_state {
  struct zn_output *output;
  struct zigzag_node *power_button;
  struct wl_event_source *second_timer_source;
  long next_sec_ms;
};

static int
zn_output_handle_second_timer(void *data)
{
  struct zigzag_layout *self = (struct zigzag_layout *)data;
  struct zn_zigzag_layout_state *state =
      (struct zn_zigzag_layout_state *)self->state;
  struct zn_server *server = zn_server_get_singleton();

  long time_ms = current_realtime_clock_ms();
  state->next_sec_ms = (time_ms - time_ms % MSEC_PER_SEC + MSEC_PER_SEC);

  int ms_delay = (int)(state->next_sec_ms - time_ms + 10);
  wl_event_source_timer_update(state->second_timer_source, ms_delay);

  struct zigzag_node *power_button = state->power_button;
  power_button->texture =
      zigzag_node_render_texture(power_button, server->renderer);
  power_button->layout->implementation->on_damage(power_button);
  return 0;
}

static void
zn_zigzag_layout_on_damage(struct zigzag_node *node)
{
  struct zn_zigzag_layout_state *state =
      (struct zn_zigzag_layout_state *)node->layout->state;
  wlr_output_damage_add_box(state->output->damage, node->frame);
}

static const struct zigzag_layout_impl zn_zigzag_layout_default_implementation =
    {
        .on_damage = zn_zigzag_layout_on_damage,
};

static void
power_button_on_click(struct zigzag_node *self, double x, double y)
{
  UNUSED(self);
  UNUSED(x);
  UNUSED(y);
  // TODO: Show "Logout"
}

static void
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
  zn_error("output: %s", output);

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

static void
power_button_set_frame(
    struct zigzag_node *self, int output_width, int output_height)
{
  double margin_height = 6;
  double margin_width = 10;
  double button_width = 70.;
  double button_height = menu_bar_height - margin_height * 2;

  self->frame->x = (double)output_width - button_width - margin_width;
  self->frame->y = (double)output_height - button_height - margin_height;
  self->frame->width = button_width;
  self->frame->height = button_height;
}

static const struct zigzag_node_impl power_button_implementation = {
    .on_click = power_button_on_click,
    .set_frame = power_button_set_frame,
    .render = power_button_render,
};

static struct zigzag_node *
create_power_button(struct zigzag_layout *node_layout, struct zn_server *server)
{
  struct zigzag_node *power_button = zigzag_node_create(
      &power_button_implementation, node_layout, NULL, server->renderer);

  if (power_button == NULL) {
    zn_error("Failed to create the VR button");
    goto err;
  }
  return power_button;
err:
  return NULL;
}

static void
menu_bar_on_click(struct zigzag_node *self, double x, double y)
{
  UNUSED(self);
  UNUSED(x);
  UNUSED(y);
}

static void
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

static void
menu_bar_set_frame(
    struct zigzag_node *self, int output_width, int output_height)
{
  double bar_width = (double)output_width;

  self->frame->x = 0.;
  self->frame->y = (double)output_height - menu_bar_height;
  self->frame->width = bar_width;
  self->frame->height = menu_bar_height;
}

static const struct zigzag_node_impl menu_bar_implementation = {
    .on_click = menu_bar_on_click,
    .set_frame = menu_bar_set_frame,
    .render = menu_bar_render,
};

static struct zigzag_node *
create_menu_bar(struct zigzag_layout *node_layout, struct zn_server *server,
    struct zigzag_node *power_button)
{
  struct zigzag_node *menu_bar = zigzag_node_create(
      &menu_bar_implementation, node_layout, NULL, server->renderer);

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
zn_zigzag_layout_setup_default_nodes(
    struct zigzag_layout *node_layout, struct zn_server *server)
{
  struct zigzag_node *power_button = create_power_button(node_layout, server);
  struct zn_zigzag_layout_state *state =
      (struct zn_zigzag_layout_state *)node_layout->state;
  state->power_button = power_button;
  struct zigzag_node *menu_bar =
      create_menu_bar(node_layout, server, power_button);
  wl_list_insert(&node_layout->nodes, &menu_bar->link);
}

struct zigzag_layout *
zn_zigzag_layout_create_default(
    struct zn_output *output, struct zn_server *server)
{
  struct zigzag_layout *self;
  struct zn_zigzag_layout_state *state;
  state = zalloc(sizeof *state);
  if (state == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }
  state->output = output;

  int output_width, output_height;
  wlr_output_transformed_resolution(
      output->wlr_output, &output_width, &output_height);

  self = zigzag_layout_create(&zn_zigzag_layout_default_implementation,
      output_width, output_height, (void *)state);

  if (self == NULL) {
    zn_error("Failed to create zigzag_layout");
    goto err_state;
  }

  zn_zigzag_layout_setup_default_nodes(self, server);

  state->second_timer_source = wl_event_loop_add_timer(
      wl_display_get_event_loop(output->wlr_output->display),
      zn_output_handle_second_timer, self);

  long time_ms = current_realtime_clock_ms();
  state->next_sec_ms = (time_ms - time_ms % MSEC_PER_SEC + MSEC_PER_SEC);

  int ms_delay = (int)(state->next_sec_ms - time_ms + 10);
  wl_event_source_timer_update(state->second_timer_source, ms_delay);

  return self;

err_state:
  free(state);
err:
  return NULL;
}

void
zn_zigzag_layout_destroy_default(struct zigzag_layout *self)
{
  struct zn_zigzag_layout_state *state =
      (struct zn_zigzag_layout_state *)self->state;
  wl_event_source_remove(state->second_timer_source);
  zigzag_node_cleanup_list(&self->nodes);
  zigzag_layout_destroy(self);
  free(state);
}
