#include "zen-desktop/screen-container.h"

#include <cglm/vec2.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen-desktop/screen-layout.h"
#include "zen-desktop/shell.h"
#include "zen/screen.h"

static void zn_screen_container_destroy(struct zn_screen_container *self);

static void
zn_screen_container_handle_screen_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_screen_container *self =
      zn_container_of(listener, self, screen_destroy_listener);

  zn_screen_container_destroy(self);
}

static void
zn_screen_container_handle_screen_resize(
    struct wl_listener *listener UNUSED, void *data UNUSED)
{
  struct zn_desktop_shell *shell = zn_desktop_shell_get_singleton();

  zn_screen_layout_reposition(shell->screen_layout);
}

struct zn_screen_container *
zn_screen_container_create(struct zn_screen *screen)
{
  struct zn_screen_container *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  glm_vec2_zero(self->position);
  wl_list_init(&self->link);
  self->screen = screen;

  self->screen_destroy_listener.notify =
      zn_screen_container_handle_screen_destroy;
  wl_signal_add(&self->screen->events.destroy, &self->screen_destroy_listener);

  self->screen_resized_listener.notify =
      zn_screen_container_handle_screen_resize;
  wl_signal_add(&self->screen->events.resized, &self->screen_resized_listener);

  return self;

err:
  return NULL;
}

static void
zn_screen_container_destroy(struct zn_screen_container *self)
{
  wl_list_remove(&self->screen_resized_listener.link);
  wl_list_remove(&self->screen_destroy_listener.link);
  wl_list_remove(&self->link);
  free(self);

  struct zn_desktop_shell *shell = zn_desktop_shell_get_singleton();
  zn_screen_layout_reposition(shell->screen_layout);
}
