#include "zen/ui/nodes/app-launcher.h"

#include <cairo.h>
#include <zigzag.h>

#include "zen-common.h"
#include "zen/ui/layout-constants.h"

static void
zn_app_launcher_on_click(struct zigzag_node *node, double x, double y)
{
  UNUSED(x);
  UNUSED(y);
  struct zn_app_launcher *app_launcher = node->user_data;
  zn_launch_command(app_launcher->data->command);
}

static bool
zn_app_launcher_render(struct zigzag_node *node, cairo_t *cr)
{
  struct zn_app_launcher *app_launcher = node->user_data;
  double icon_width =
      cairo_image_surface_get_width(app_launcher->launcher_icon_surface);
  double icon_height =
      cairo_image_surface_get_height(app_launcher->launcher_icon_surface);

  double icon_x = (launcher_width - icon_width) / 2;
  double icon_y = (menu_bar_height - icon_height) / 2;

  cairo_set_source_surface(
      cr, app_launcher->launcher_icon_surface, icon_x, icon_y);
  cairo_paint(cr);

  return true;
}

static const struct zigzag_node_impl implementation = {
    .on_click = zn_app_launcher_on_click,
    .render = zn_app_launcher_render,
};

static void
zn_app_launcher_init_frame(struct zigzag_node *node, double screen_height)
{
  struct zn_app_launcher *app_launcher = node->user_data;
  node->pending.frame.x =
      launcher_margin_left + app_launcher->idx * launcher_width;
  node->pending.frame.y = screen_height - menu_bar_height;
  node->pending.frame.width = launcher_width;
  node->pending.frame.height = menu_bar_height;
}

struct zn_app_launcher *
zn_app_launcher_create(struct zigzag_layout *zigzag_layout,
    const struct zn_app_launcher_data *data, int idx)
{
  struct zn_app_launcher *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }
  self->data = data;
  self->idx = idx;

  self->launcher_icon_surface =
      cairo_image_surface_create_from_png(self->data->icon_path);
  if (cairo_surface_status(self->launcher_icon_surface) !=
      CAIRO_STATUS_SUCCESS) {
        zn_error("Failed to create png surface: %s", app->icon);
    goto err_cairo_surface;
  }

  struct zigzag_node *zigzag_node =
      zigzag_node_create(&implementation, zigzag_layout, true, self);

  if (zigzag_node == NULL) {
    zn_error("Failed to create a zigzag_node");
    goto err_cairo_surface;
  }
  self->zigzag_node = zigzag_node;

  wl_list_init(&self->link);

  zn_app_launcher_init_frame(self->zigzag_node, zigzag_layout->screen_height);

  return self;

err_cairo_surface:
  cairo_surface_destroy(self->launcher_icon_surface);
  free(self);

err:
  return NULL;
}

void
zn_app_launcher_destroy(struct zn_app_launcher *self)
{
  wl_list_remove(&self->link);
  cairo_surface_destroy(self->launcher_icon_surface);
  zigzag_node_destroy(self->zigzag_node);
  free(self);
}
