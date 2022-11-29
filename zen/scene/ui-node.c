/*
TODO:
- Make cairo directly touch the wlr texture
- Need to think about re-rendering
  - What is updated on the click event?
    -> Directly update the wlr texture?
  - Add updateState(callback)
    - update the state and needed post- and pre- processing properly
  - What do we need to do every frame?
  - How to avoild wlr_render_start()?
- Only render the ui nodes in one screen
*/
#include "zen/scene/ui-node.h"

#include <cairo.h>
#include <drm_fourcc.h>
#include <librsvg/rsvg.h>
#include <wayland-server-core.h>

#include "build-config.h"
#include "zen-common.h"
#include "zen/cairo/texture.h"
#include "zen/output.h"

struct wlr_texture *
zn_ui_node_render_texture(struct zn_ui_node *self, struct zn_server *server)
{
  struct wlr_texture *texture = NULL;
  cairo_surface_t *surface = cairo_image_surface_create(
      CAIRO_FORMAT_RGB24, self->frame->width, self->frame->height);

  if (!zn_assert(cairo_surface_status(surface) == CAIRO_STATUS_SUCCESS,
          "Failed to create cairo_surface"))
    goto err_cairo_surface;

  cairo_t *cr = cairo_create(surface);
  if (!zn_assert(
          cairo_status(cr) == CAIRO_STATUS_SUCCESS, "Failed to create cairo_t"))
    goto err_cairo;

  self->renderer(self, cr);

  texture = zn_wlr_texture_from_cairo_surface(surface, server);
  struct wlr_fbox fframe = {.x = (double)self->frame->x,
      .y = (double)self->frame->y,
      .width = (double)self->frame->width,
      .height = (double)self->frame->height};
  zn_output_add_damage_box(self->screen->output, &fframe);
err_cairo:
  cairo_destroy(cr);
err_cairo_surface:
  cairo_surface_destroy(surface);

  return texture;
}

void
zn_ui_node_update_texture(struct zn_ui_node *self)
{
  struct wlr_texture *old_texture = self->texture;
  self->texture = zn_ui_node_render_texture(self, zn_server_get_singleton());
  wlr_texture_destroy(old_texture);
}

struct zn_ui_node *
zn_ui_node_create(struct zn_screen *screen, struct wlr_box *frame, void *data,
    struct zn_server *server, zn_ui_node_on_click_handler_t on_click_handler,
    zn_ui_node_render_t renderer)
{
  struct zn_ui_node *self;
  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }
  wl_list_init(&self->children);
  self->screen = screen;
  self->frame = frame;
  self->on_click_handler = on_click_handler;
  self->renderer = renderer;
  self->data = data;

  self->texture = zn_ui_node_render_texture(self, server);

err:
  return self;
}

void
zn_ui_node_destroy(struct zn_ui_node *self)
{
  wl_list_remove(&self->children);
  wlr_texture_destroy(self->texture);
  if (self->data) free(self->data);
  free(self->frame);
  free(self);
}

void
vr_button_on_click(struct zn_ui_node *self, double x, double y)
{
  UNUSED(self);
  UNUSED(x);
  UNUSED(y);
  zn_warn("VR Mode starting...");
}

void
vr_button_render(struct zn_ui_node *self, cairo_t *cr)
{
  // TODO: With cairo_in_fill(), non-rectangle clickable area can be created
  // TODO: use color code
  cairo_set_source_rgb(cr, 0.067, 0.122, 0.302);
  zn_cairo_draw_rounded_rectangle(
      cr, self->frame->width, self->frame->height, self->frame->height / 2);
  cairo_fill(cr);

  // TODO: verify the font
  cairo_select_font_face(
      cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, 18);
  cairo_set_source_rgb(cr, 0.953, 0.957, 0.965);
  zn_cairo_draw_centered_text(
      cr, "VR", self->frame->width, self->frame->height);
}

struct zn_ui_node *
create_vr_button(struct zn_screen *screen, struct zn_server *server,
    int output_width, int output_height)
{
  double button_width = 160;
  double button_height = 40;

  struct wlr_box *frame;
  frame = zalloc(sizeof *frame);

  // Put the frame to the center of screen
  frame->x = (double)output_width / 2 - button_width / 2;
  frame->y = (double)output_height - button_height - 10.;
  frame->width = button_width;
  frame->height = button_height;

  struct zn_ui_node *vr_button = zn_ui_node_create(
      screen, frame, NULL, server, vr_button_on_click, vr_button_render);

  if (vr_button == NULL) {
    zn_error("Failed to create the VR button");
    goto err;
  }
  return vr_button;
err:
  free(frame);
  return NULL;
}

void
power_menu_quit_on_click(struct zn_ui_node *self, double x, double y)
{
  UNUSED(x);
  UNUSED(y);
  UNUSED(self);
  struct zn_server *server = zn_server_get_singleton();
  zn_server_terminate(server, EXIT_SUCCESS);
}

void
power_menu_quit_render(struct zn_ui_node *self, cairo_t *cr)
{
  cairo_set_source_rgb(cr, 0.812, 0.824, 0.859);
  cairo_paint(cr);

  cairo_select_font_face(
      cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, 18);
  cairo_set_source_rgb(cr, 0.102, 0.102, 0.102);
  zn_cairo_draw_left_aligned_text(
      cr, "Quit", self->frame->width, self->frame->height, 10);

  cairo_set_source_rgb(cr, 0.502, 0.502, 0.502);
  zn_cairo_draw_right_aligned_text(
      cr, "alt + q", self->frame->width, self->frame->height, 10);
}

struct zn_ui_node *
create_power_menu_quit(struct zn_screen *screen, struct zn_server *server,
    int output_width, int output_height)
{
  double menu_width = 180;
  double menu_height = 30;
  struct wlr_box *frame;
  frame = zalloc(sizeof *frame);

  // Put the frame to the center of screen
  frame->x = (double)output_width - menu_width;
  frame->y = (double)output_height - menu_height - 60;
  frame->width = menu_width;
  frame->height = menu_height;

  struct zn_ui_node *power_menu_quit = zn_ui_node_create(screen, frame, NULL,
      server, power_menu_quit_on_click, power_menu_quit_render);

  if (power_menu_quit == NULL) {
    zn_error("Failed to create the power button");
    goto err;
  }
  return power_menu_quit;
err:
  free(frame);
  return NULL;
}

struct power_button_state {
  bool clicked;
};

void
power_button_on_click(struct zn_ui_node *self, double x, double y)
{
  UNUSED(x);
  UNUSED(y);
  struct power_button_state *state = (struct power_button_state *)self->data;
  state->clicked = !state->clicked;
  zn_ui_node_update_texture(self);
}

void
power_button_render(struct zn_ui_node *self, cairo_t *cr)
{
  GError *error = NULL;
  struct power_button_state *state = (struct power_button_state *)self->data;
  GFile *file = g_file_new_for_path(
      state->clicked ? POWER_BUTTON_ICON_CLICKED : POWER_BUTTON_ICON);
  RsvgHandle *handle = rsvg_handle_new_from_gfile_sync(
      file, RSVG_HANDLE_FLAGS_NONE, NULL, &error);
  if (handle == NULL) {
    zn_error("Failed to create the svg handler: %s", error->message);
    goto err;
  }
  RsvgRectangle viewport = {
      .x = 0.0,
      .y = 0.0,
      .width = self->frame->width,
      .height = self->frame->height,
  };
  if (!rsvg_handle_render_document(handle, cr, &viewport, &error)) {
    zn_error("Failed to render the svg");
  }

  g_object_unref(handle);
err:
  return;
}

struct zn_ui_node *
create_power_button(struct zn_screen *screen, struct zn_server *server,
    int output_width, int output_height, struct zn_ui_node *power_menu_quit)
{
  double button_width = 40;
  double button_height = 40;
  struct wlr_box *frame;
  frame = zalloc(sizeof *frame);

  // Put the frame to the center of screen
  frame->x = (double)output_width - button_width - 10;
  frame->y = (double)output_height - button_height - 10;
  frame->width = button_width;
  frame->height = button_height;

  struct power_button_state *state;
  state = zalloc(sizeof *state);
  state->clicked = false;

  struct zn_ui_node *power_button = zn_ui_node_create(screen, frame,
      (void *)state, server, power_button_on_click, power_button_render);

  if (power_button == NULL) {
    zn_error("Failed to create the power button");
    goto err;
  }
  wl_list_insert(&power_button->children, &power_menu_quit->link);
  return power_button;
err:
  free(frame);
  return NULL;
}

void
menu_bar_on_click(struct zn_ui_node *self, double x, double y)
{
  UNUSED(self);
  UNUSED(x);
  UNUSED(y);
}

void
menu_bar_render(struct zn_ui_node *self, cairo_t *cr)
{
  UNUSED(self);
  cairo_set_source_rgb(cr, 0.529, 0.557, 0.647);
  cairo_paint(cr);
}

struct zn_ui_node *
create_menu_bar(struct zn_screen *screen, struct zn_server *server,
    int output_width, int output_height, struct zn_ui_node *vr_button,
    struct zn_ui_node *power_button)
{
  double bar_width = (double)output_width;
  double bar_height = 60;
  struct wlr_box *frame;
  frame = zalloc(sizeof *frame);

  // Put the frame to the center of screen
  frame->x = 0.;
  frame->y = (double)output_height - bar_height;
  frame->width = bar_width;
  frame->height = bar_height;

  struct zn_ui_node *menu_bar = zn_ui_node_create(
      screen, frame, NULL, server, menu_bar_on_click, menu_bar_render);

  if (menu_bar == NULL) {
    zn_error("Failed to create the menu bar");
    goto err;
  }
  wl_list_insert(&menu_bar->children, &vr_button->link);
  wl_list_insert(&menu_bar->children, &power_button->link);
  return menu_bar;
err:
  free(frame);
  return NULL;
}

void
zn_ui_node_setup_default(struct zn_screen *screen, struct zn_server *server)
{
  int output_width, output_height;
  wlr_output_transformed_resolution(
      screen->output->wlr_output, &output_width, &output_height);
  struct zn_ui_node *vr_button =
      create_vr_button(screen, server, output_width, output_height);
  struct zn_ui_node *power_menu_quit =
      create_power_menu_quit(screen, server, output_width, output_height);
  struct zn_ui_node *power_button = create_power_button(
      screen, server, output_width, output_height, power_menu_quit);
  struct zn_ui_node *menu_bar = create_menu_bar(
      screen, server, output_width, output_height, vr_button, power_button);
  // Register the widgets on the screen
  wl_list_insert(&screen->ui_nodes, &menu_bar->link);
}