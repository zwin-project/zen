#include "zen/cursor.h"

#include <drm/drm_fourcc.h>
#include <linux/input.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_output_damage.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/xcursor.h>

#include "zen-common.h"
#include "zen/input/cursor-grab.h"
#include "zen/scene/screen-layout.h"
#include "zen/scene/view.h"
#include "zen/server.h"

static void
default_grab_motion(
    struct zn_cursor_grab* grab, struct wlr_event_pointer_motion* event)
{
  struct zn_server* server = zn_server_get_singleton();
  struct wlr_seat* seat = server->input_manager->seat->wlr_seat;
  struct zn_cursor* cursor = grab->cursor;
  struct wlr_surface* surface;
  struct zn_view* view;
  double view_x, view_y;

  if (cursor->screen == NULL) {
    return;
  }

  view = zn_screen_get_view_at(
      cursor->screen, cursor->x, cursor->y, &view_x, &view_y);

  if (view != NULL) {
    surface = view->impl->get_wlr_surface(view);
  } else {
    surface = NULL;
  }

  if (surface) {
    wlr_seat_pointer_enter(seat, surface, view_x, view_y);
    wlr_seat_pointer_send_motion(seat, event->time_msec, view_x, view_y);
  } else {
    zn_cursor_set_xcursor(cursor, "left_ptr");
    wlr_seat_pointer_clear_focus(seat);
  }
}

static void
default_grab_button(
    struct zn_cursor_grab* grab, struct wlr_event_pointer_button* event)
{
  struct zn_server* server = zn_server_get_singleton();
  struct wlr_seat* seat = server->input_manager->seat->wlr_seat;
  struct zn_cursor* cursor = grab->cursor;
  struct zn_view* view;

  if (cursor->screen == NULL) {
    return;
  }

  wlr_seat_pointer_send_button(
      seat, event->time_msec, event->button, event->state);

  if (event->state == WLR_BUTTON_PRESSED) {
    view =
        zn_screen_get_view_at(cursor->screen, cursor->x, cursor->y, NULL, NULL);
    zn_scene_set_focused_view(server->scene, view);
  }
}

static void
default_grab_axis(
    struct zn_cursor_grab* grab, struct wlr_event_pointer_axis* event)
{
  UNUSED(grab);
  struct zn_server* server = zn_server_get_singleton();
  struct wlr_seat* seat = server->input_manager->seat->wlr_seat;

  wlr_seat_pointer_send_axis(seat, event->time_msec, event->orientation,
      event->delta, event->delta_discrete, event->source);
}

static void
default_grab_frame(struct zn_cursor_grab* grab)
{
  UNUSED(grab);
  struct zn_server* server = zn_server_get_singleton();
  struct wlr_seat* seat = server->input_manager->seat->wlr_seat;

  wlr_seat_pointer_send_frame(seat);
}

static const struct zn_cursor_grab_interface default_grab_interface = {
    .motion = default_grab_motion,
    .button = default_grab_button,
    .axis = default_grab_axis,
    .frame = default_grab_frame,
};

static void
zn_cursor_update_size(struct zn_cursor* self)
{
  if (!self->texture) {
    self->width = 0;
    self->height = 0;
  } else if (self->surface) {
    self->width = self->surface->current.width;
    self->height = self->surface->current.height;
  } else {
    self->width = self->texture->width;
    self->height = self->texture->height;
  }
}

// screen_x and screen_y must be less than screen->width/height
static void
zn_cursor_update_position(struct zn_cursor* self, struct zn_screen* screen,
    double screen_x, double screen_y)
{
  self->x = screen_x;
  self->y = screen_y;

  if (self->screen == screen) {
    return;
  }

  if (self->screen != NULL) {
    wl_list_remove(&self->screen_destroy_listener.link);
    wl_list_init(&self->screen_destroy_listener.link);
  }
  if (screen != NULL) {
    wl_signal_add(&screen->events.destroy, &self->screen_destroy_listener);
  }

  self->screen = screen;
}

static void
zn_cursor_damage_whole(struct zn_cursor* self)
{
  struct wlr_fbox fbox;

  if (self->screen) {
    zn_cursor_get_fbox(self, &fbox);
    zn_output_add_damage_box(self->screen->output, &fbox);
  }
}

static void
zn_cursor_handle_new_screen(struct wl_listener* listener, void* data)
{
  struct zn_cursor* self = zn_container_of(listener, self, new_screen_listener);
  struct zn_screen* screen = data;
  struct wlr_fbox box;

  if (self->screen == NULL) {
    zn_screen_get_fbox(screen, &box);
    zn_cursor_update_position(self, screen, box.width / 2, box.height / 2);
    zn_cursor_damage_whole(self);
  }
}

static void
zn_cursor_handle_screen_destroy(struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zn_cursor* self =
      zn_container_of(listener, self, screen_destroy_listener);
  struct zn_server* server = zn_server_get_singleton();
  struct zn_screen_layout* screen_layout = server->scene->screen_layout;
  struct zn_screen* screen;
  struct wlr_fbox box = {0};
  bool found = false;

  wl_list_for_each(screen, &screen_layout->screens, link)
  {
    if (screen != self->screen) {
      found = true;
      break;
    }
  }

  if (found) {
    zn_screen_get_fbox(self->screen, &box);
    zn_cursor_update_position(self, screen, box.width / 2, box.height / 2);
    zn_cursor_damage_whole(self);
  } else {
    zn_cursor_update_position(self, NULL, 0, 0);
  }
}

static void
zn_cursor_handle_surface_commit(struct wl_listener* listener, void* data)
{
  struct zn_cursor* self =
      zn_container_of(listener, self, surface_commit_listener);
  UNUSED(data);

  if (!zn_assert(
          self->surface, "Handling surface commit while no surface exists")) {
    return;
  }

  zn_cursor_damage_whole(self);

  zn_cursor_update_size(self);

  zn_cursor_damage_whole(self);
}

static void
zn_cursor_handle_surface_destroy(struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zn_cursor* self =
      zn_container_of(listener, self, surface_destroy_listener);

  zn_cursor_set_xcursor(self, "left_ptr");
}

void
zn_cursor_move_relative(struct zn_cursor* self, double dx, double dy)
{
  struct zn_server* server = zn_server_get_singleton();
  struct zn_screen_layout* layout = server->scene->screen_layout;
  struct zn_screen* new_screen;
  double layout_x, layout_y;
  double screen_x = self->x + dx;
  double screen_y = self->y + dy;

  if (self->screen == NULL) {
    return;
  }

  zn_screen_get_screen_layout_coords(
      self->screen, screen_x, screen_y, &layout_x, &layout_y);

  new_screen = zn_screen_layout_get_closest_screen(
      layout, layout_x, layout_y, &screen_x, &screen_y);

  zn_cursor_damage_whole(self);

  zn_cursor_update_position(self, new_screen, screen_x, screen_y);

  zn_cursor_damage_whole(self);
}

void
zn_cursor_get_fbox(struct zn_cursor* self, struct wlr_fbox* fbox)
{
  fbox->x = self->x - self->hotspot_x;
  fbox->y = self->y - self->hotspot_y;
  fbox->width = self->width;
  fbox->height = self->height;
}

void
zn_cursor_set_surface(struct zn_cursor* self, struct wlr_surface* surface,
    int hotspot_x, int hotspot_y)
{
  zn_cursor_damage_whole(self);

  if (self->texture && !self->surface) {
    wlr_texture_destroy(self->texture);
  }

  if (self->surface != NULL) {
    wl_list_remove(&self->surface_destroy_listener.link);
    wl_list_init(&self->surface_destroy_listener.link);
    wl_list_remove(&self->surface_commit_listener.link);
    wl_list_init(&self->surface_commit_listener.link);
  }

  if (surface != NULL) {
    wl_signal_add(&surface->events.destroy, &self->surface_destroy_listener);
    wl_signal_add(&surface->events.commit, &self->surface_commit_listener);

    self->hotspot_x = hotspot_x;
    self->hotspot_y = hotspot_y;
    self->texture = wlr_surface_get_texture(surface);
  } else {
    self->hotspot_x = 0;
    self->hotspot_y = 0;
    self->texture = NULL;
  }

  self->surface = surface;

  zn_cursor_update_size(self);

  zn_cursor_damage_whole(self);
}

void
zn_cursor_set_xcursor(struct zn_cursor* self, char* name)
{
  struct zn_server* server = zn_server_get_singleton();
  struct wlr_xcursor* xcursor;
  struct wlr_xcursor_image* image;

  zn_cursor_set_surface(self, NULL, 0, 0);

  xcursor = wlr_xcursor_manager_get_xcursor(self->xcursor_manager, name, 1.f);
  if (xcursor == NULL) {
    zn_error("Failed to get xcursor");
    return;
  }
  image = xcursor->images[0];

  zn_cursor_damage_whole(self);

  self->hotspot_x = image->hotspot_x;
  self->hotspot_y = image->hotspot_y;
  self->texture = wlr_texture_from_pixels(server->renderer, DRM_FORMAT_ARGB8888,
      image->width * 4, image->width, image->height, image->buffer);

  zn_cursor_update_size(self);

  zn_cursor_damage_whole(self);
}

struct zn_cursor*
zn_cursor_create(void)
{
  struct zn_server* server = zn_server_get_singleton();
  struct zn_screen_layout* screen_layout = server->scene->screen_layout;
  struct zn_cursor* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->xcursor_manager = wlr_xcursor_manager_create(NULL, 24);
  if (self->xcursor_manager == NULL) {
    zn_error("Failed to create wlr_xcursor_manager");
    goto err_free;
  }
  wlr_xcursor_manager_load(self->xcursor_manager, 1.f);

  self->grab_default.interface = &default_grab_interface;
  self->grab_default.cursor = self;
  self->grab = &self->grab_default;
  self->screen = NULL;

  zn_cursor_set_xcursor(self, "left_ptr");
  zn_cursor_update_size(self);

  self->new_screen_listener.notify = zn_cursor_handle_new_screen;
  wl_signal_add(&screen_layout->events.new_screen, &self->new_screen_listener);

  self->screen_destroy_listener.notify = zn_cursor_handle_screen_destroy;
  wl_list_init(&self->screen_destroy_listener.link);

  self->surface_commit_listener.notify = zn_cursor_handle_surface_commit;
  wl_list_init(&self->surface_commit_listener.link);

  self->surface_destroy_listener.notify = zn_cursor_handle_surface_destroy;
  wl_list_init(&self->surface_destroy_listener.link);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_cursor_destroy(struct zn_cursor* self)
{
  wl_list_remove(&self->new_screen_listener.link);
  wl_list_remove(&self->screen_destroy_listener.link);
  wl_list_remove(&self->surface_commit_listener.link);
  wl_list_remove(&self->surface_destroy_listener.link);
  if (self->texture && !self->surface) {
    wlr_texture_destroy(self->texture);
  }
  wlr_xcursor_manager_destroy(self->xcursor_manager);
  free(self);
}
