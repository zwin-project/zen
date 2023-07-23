#include "zen-desktop/screen.h"

#include <cglm/vec2.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen-desktop/screen-layout.h"
#include "zen-desktop/shell.h"
#include "zen/screen.h"
#include "zen/snode-root.h"
#include "zen/snode.h"

static void zn_desktop_screen_destroy(struct zn_desktop_screen *self);

static void
zn_desktop_screen_handle_screen_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_desktop_screen *self =
      zn_container_of(listener, self, screen_destroy_listener);

  // TODO(@Aki-7): move views and cursor if exists

  zn_desktop_screen_destroy(self);
}

static void
zn_desktop_screen_handle_screen_resize(
    struct wl_listener *listener UNUSED, void *data UNUSED)
{
  struct zn_desktop_shell *shell = zn_desktop_shell_get_singleton();

  zn_screen_layout_reposition(shell->screen_layout);
}

void
zn_desktop_screen_set_position(struct zn_desktop_screen *self, vec2 position)
{
  zn_screen_set_layout_position(self->screen, position);
}

void
zn_desktop_screen_effective_to_layout_coords(
    struct zn_desktop_screen *self, vec2 effective, vec2 layout)
{
  glm_vec2_add(self->screen->layout_position, effective, layout);
}

struct zn_desktop_screen *
zn_desktop_screen_get(struct zn_screen *screen)
{
  return screen->user_data;
}

struct zn_desktop_screen *
zn_desktop_screen_create(struct zn_screen *screen)
{
  struct zn_desktop_screen *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wl_list_init(&self->link);
  self->screen = screen;
  screen->user_data = self;

  self->cursor_layer = zn_snode_create(self, &zn_snode_noop_implementation);
  if (self->cursor_layer == NULL) {
    zn_error("Failed to create a snode");
    goto err_free;
  }

  self->view_layer = zn_snode_create(self, &zn_snode_noop_implementation);
  if (self->view_layer == NULL) {
    zn_error("Failed to create a snode");
    goto err_cursor_layer;
  }

  zn_snode_set_position(screen->layers[ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND],
      screen->snode_root->node, GLM_VEC2_ZERO);
  zn_snode_set_position(screen->layers[ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM],
      screen->snode_root->node, GLM_VEC2_ZERO);
  zn_snode_set_position(
      self->view_layer, screen->snode_root->node, GLM_VEC2_ZERO);
  zn_snode_set_position(screen->layers[ZWLR_LAYER_SHELL_V1_LAYER_TOP],
      screen->snode_root->node, GLM_VEC2_ZERO);
  zn_snode_set_position(screen->layers[ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY],
      screen->snode_root->node, GLM_VEC2_ZERO);
  zn_snode_set_position(
      self->cursor_layer, screen->snode_root->node, GLM_VEC2_ZERO);

  self->screen_destroy_listener.notify =
      zn_desktop_screen_handle_screen_destroy;
  wl_signal_add(&self->screen->events.destroy, &self->screen_destroy_listener);

  self->screen_resized_listener.notify = zn_desktop_screen_handle_screen_resize;
  wl_signal_add(&self->screen->events.resized, &self->screen_resized_listener);

  return self;

err_cursor_layer:
  zn_snode_destroy(self->cursor_layer);

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_desktop_screen_destroy(struct zn_desktop_screen *self)
{
  wl_list_remove(&self->screen_resized_listener.link);
  wl_list_remove(&self->screen_destroy_listener.link);
  wl_list_remove(&self->link);
  zn_snode_destroy(self->view_layer);
  zn_snode_destroy(self->cursor_layer);
  free(self);

  struct zn_desktop_shell *shell = zn_desktop_shell_get_singleton();
  zn_screen_layout_reposition(shell->screen_layout);
}
