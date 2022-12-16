#include "zen/cursor.h"

#include <cglm/quat.h>
#include <cglm/vec2.h>
#include <cglm/vec3.h>
#include <drm_fourcc.h>
#include <zen-common.h>

#include "zen/appearance/cursor.h"
#include "zen/board.h"
#include "zen/screen.h"
#include "zen/screen/cursor-grab/default.h"
#include "zen/server.h"

static void
zn_cursor_handle_board_destroy(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_cursor *self =
      zn_container_of(listener, self, board_destroy_listener);

  zn_cursor_move(self, NULL, 0, 0);

  zna_cursor_commit(self->appearance, ZNA_CURSOR_DAMAGE_GEOMETRY);

  // TODO: show cursor on another board when screen display system mode
  // TODO: handle the case the board becomes invisible but not destroyed
}

static void
zn_cursor_handle_surface_commit(struct wl_listener *listener, void *data)
{
  struct zn_cursor *self =
      zn_container_of(listener, self, surface_commit_listener);
  UNUSED(data);

  if (!zn_assert(
          self->surface, "Handling surface commit while no surface exists")) {
    return;
  }

  self->visible = wlr_surface_has_buffer(self->surface);

  zn_cursor_damage(self);
}

static void
zn_cursor_handle_surface_destroy(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_cursor *self =
      zn_container_of(listener, self, surface_destroy_listener);

  zn_cursor_set_surface(self, NULL, 0, 0);
}

void
zn_cursor_get_fbox(struct zn_cursor *self, struct wlr_fbox *fbox)
{
  fbox->x = self->x;
  fbox->y = self->y;

  if (!self->visible) {
    fbox->width = 0;
    fbox->height = 0;
  } else if (self->surface) {
    fbox->x -= self->surface_hotspot_x;
    fbox->y -= self->surface_hotspot_y;
    fbox->width = self->surface->current.width;
    fbox->height = self->surface->current.height;
  } else if (self->xcursor_texture) {
    fbox->x -= self->xcursor_hotspot_x;
    fbox->y -= self->xcursor_hotspot_y;
    fbox->width = self->xcursor_texture->width;
    fbox->height = self->xcursor_texture->height;
  }
}

void
zn_cursor_damage(struct zn_cursor *self)
{
  struct wlr_fbox fbox;

  if (self->board == NULL || self->board->screen == NULL) return;

  zn_cursor_get_fbox(self, &fbox);
  zn_screen_damage(self->board->screen, &fbox);
}

struct wlr_texture *
zn_cursor_get_texture(struct zn_cursor *self)
{
  if (self->surface) {
    return wlr_surface_get_texture(self->surface);
  }
  return self->xcursor_texture;
}

void
zn_cursor_set_surface(struct zn_cursor *self, struct wlr_surface *surface,
    int hotspot_x, int hotspot_y)
{
  if (self->surface != NULL) {
    wl_list_remove(&self->surface_destroy_listener.link);
    wl_list_init(&self->surface_destroy_listener.link);
    wl_list_remove(&self->surface_commit_listener.link);
    wl_list_init(&self->surface_commit_listener.link);
  }

  if (surface != NULL) {
    wl_signal_add(&surface->events.destroy, &self->surface_destroy_listener);
    wl_signal_add(&surface->events.commit, &self->surface_commit_listener);
  }

  zn_cursor_damage(self);

  self->surface_hotspot_x = hotspot_x;
  self->surface_hotspot_y = hotspot_y;
  self->visible = surface != NULL;
  self->surface = surface;

  zn_cursor_damage(self);
  zna_cursor_commit(self->appearance, ZNA_CURSOR_DAMAGE_TEXTURE);
}

void
zn_cursor_set_xcursor(struct zn_cursor *self, const char *name)
{
  // TODO: Consider the cursor image specified by the client.
  struct zn_server *server = zn_server_get_singleton();
  struct wlr_xcursor *xcursor = NULL;

  if (self->surface) {
    zn_cursor_set_surface(self, NULL, 0, 0);
  }

  if (strcmp(name, self->xcursor_name) == 0) {
    self->visible = true;
    return;
  }

  if (strlen(name) > 0) {
    xcursor = wlr_xcursor_manager_get_xcursor(self->xcursor_manager, name, 1.f);
    if (xcursor == NULL) {
      zn_warn("cursor %s could not found", name);
      return;
    }
  }

  zn_cursor_damage(self);

  if (self->xcursor_texture) {
    wlr_texture_destroy(self->xcursor_texture);
    self->xcursor_texture = NULL;
    self->visible = false;
  }

  if (xcursor) {
    struct wlr_xcursor_image *image = xcursor->images[0];
    self->xcursor_texture =
        wlr_texture_from_pixels(server->renderer, DRM_FORMAT_ARGB8888,
            image->width * 4, image->width, image->height, image->buffer);
    self->xcursor_hotspot_x = image->hotspot_x;
    self->xcursor_hotspot_y = image->hotspot_y;
    self->visible = true;
  }

  free(self->xcursor_name);
  self->xcursor_name = strdup(name);

  zn_cursor_damage(self);
  zna_cursor_commit(self->appearance, ZNA_CURSOR_DAMAGE_TEXTURE);
}

void
zn_cursor_move(
    struct zn_cursor *self, struct zn_board *board, double x, double y)
{
  if (self->board) {
    wl_list_remove(&self->board_destroy_listener.link);
    wl_list_init(&self->board_destroy_listener.link);
  }

  if (board) {
    wl_signal_add(&board->events.destroy, &self->board_destroy_listener);
  }

  zn_cursor_damage(self);

  self->x = x;
  self->y = y;

  self->board = board;

  zn_cursor_damage(self);

  if (self->board) {
    struct wlr_fbox cursor_fbox, board_local_cursor_geom;

    zn_cursor_get_fbox(self, &cursor_fbox);
    zn_board_box_effective_to_local_geom(
        self->board, &cursor_fbox, &board_local_cursor_geom);

    self->geometry.size[0] = board_local_cursor_geom.width;
    self->geometry.size[1] = board_local_cursor_geom.height;

    glm_mat4_copy(self->board->geometry.transform, self->geometry.transform);
    glm_translate(self->geometry.transform,
        (vec3){board_local_cursor_geom.x, board_local_cursor_geom.y,
            CURSOR_Z_OFFSET_ON_BOARD});
  } else {
    glm_vec2_zero(self->geometry.size);
    glm_mat4_identity(self->geometry.transform);
  }
}

static bool
zn_cursor_is_default_grab(struct zn_cursor *self)
{
  if (self->grab == NULL || self->default_grab == NULL) return false;

  return self->grab->impl == self->default_grab->base.impl;
}

void
zn_cursor_start_grab(struct zn_cursor *self, struct zn_cursor_grab *grab)
{
  if (!zn_assert(
          zn_cursor_is_default_grab(self), "Non-default grab already exists")) {
    return;
  }

  self->grab->impl->cancel(self->grab);

  self->grab = grab;
  self->grab->cursor = self;

  self->grab->impl->rebase(self->grab);
}

void
zn_cursor_end_grab(struct zn_cursor *self)
{
  self->grab->impl->cancel(self->grab);

  self->grab = &self->default_grab->base;
  self->grab->cursor = self;

  self->grab->impl->rebase(self->grab);
}

struct zn_cursor *
zn_cursor_create(void)
{
  struct zn_cursor *self;
  struct zn_server *server = zn_server_get_singleton();

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

  self->appearance = zna_cursor_create(self, server->appearance_system);
  if (self->appearance == NULL) {
    zn_error("Failed to create a zna_cursor");
    goto err_xcursor_manager;
  }

  self->x = 0;
  self->y = 0;
  self->board = NULL;

  self->xcursor_name = strdup("");
  self->xcursor_texture = NULL;
  self->surface_hotspot_x = 0;
  self->surface_hotspot_y = 0;

  glm_vec2_zero(self->geometry.size);
  glm_mat4_identity(self->geometry.transform);

  self->board_destroy_listener.notify = zn_cursor_handle_board_destroy;
  wl_list_init(&self->board_destroy_listener.link);

  self->surface_commit_listener.notify = zn_cursor_handle_surface_commit;
  wl_list_init(&self->surface_commit_listener.link);

  self->surface_destroy_listener.notify = zn_cursor_handle_surface_destroy;
  wl_list_init(&self->surface_destroy_listener.link);

  zn_cursor_set_xcursor(self, "left_ptr");

  self->default_grab = zn_default_cursor_grab_create();
  self->grab = &self->default_grab->base;
  self->grab->cursor = self;

  return self;

err_xcursor_manager:
  wlr_xcursor_manager_destroy(self->xcursor_manager);

err_free:
  free(self);

err:
  return NULL;
}

void
zn_cursor_destroy_resources(struct zn_cursor *self)
{
  if (self->grab) self->grab->impl->cancel(self->grab);
  self->grab = NULL;

  if (self->xcursor_texture) {
    wlr_texture_destroy(self->xcursor_texture);
  }
}

void
zn_cursor_destroy(struct zn_cursor *self)
{
  wl_list_remove(&self->board_destroy_listener.link);
  wl_list_remove(&self->surface_commit_listener.link);
  wl_list_remove(&self->surface_destroy_listener.link);
  zn_default_cursor_grab_destroy(self->default_grab);
  zna_cursor_destroy(self->appearance);
  wlr_xcursor_manager_destroy(self->xcursor_manager);
  free(self->xcursor_name);
  free(self);
}
