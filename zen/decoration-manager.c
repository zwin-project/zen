#include "zen/decoration-manager.h"

#include "zen-common.h"
#include "zen/xdg-decoration.h"

static void
zn_decoration_manager_handle_new_xdg_decoration(
    struct wl_listener* listener, void* data)
{
  struct zn_decoration_manager* self =
      zn_container_of(listener, self, new_xdg_decoration_listener);
  struct wlr_xdg_toplevel_decoration_v1* decoration = data;
  zn_xdg_decoration_create(self, decoration);
}

struct zn_decoration_manager*
zn_decoration_manager_create(struct wl_display* display)
{
  struct zn_decoration_manager* self;
  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->decoration_manager = wlr_server_decoration_manager_create(display);
  if (self->decoration_manager == NULL) {
    zn_error("Failed to create wlr_server_decoration_manager");
    goto err_free;
  }

  self->xdg_decoration_manager = wlr_xdg_decoration_manager_v1_create(display);
  if (self->xdg_decoration_manager == NULL) {
    zn_error("Failed to create wlr_xdg_decoration_manager");
    goto err_free;
  }

  wlr_server_decoration_manager_set_default_mode(
      self->decoration_manager, WLR_SERVER_DECORATION_MANAGER_MODE_CLIENT);

  wl_list_init(&self->xdg_decoration_list);

  self->new_xdg_decoration_listener.notify =
      zn_decoration_manager_handle_new_xdg_decoration;
  wl_signal_add(&self->xdg_decoration_manager->events.new_toplevel_decoration,
      &self->new_xdg_decoration_listener);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_decoration_manager_destroy(struct zn_decoration_manager* self)
{
  struct zn_xdg_decoration *deco, *tmp;
  wl_list_for_each_safe (deco, tmp, &self->xdg_decoration_list, link) {
    zn_xdg_decoration_destory(deco);
  }

  free(self);
}
