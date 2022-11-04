#include "appearance.h"

#include <zen-common.h>
#include <zgnr/rendering-unit.h>

#include "scene/virtual-object/rendering-unit.h"

static void
zn_appearance_handle_new_rendering_unit(
    struct wl_listener* listener, void* data)
{
  UNUSED(listener);
  struct zgnr_rendering_unit* unit = data;

  (void)zn_rendering_unit_create(unit);
}

struct zn_appearance*
zn_appearance_create(struct wl_display* display)
{
  struct zn_appearance* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->display = display;

  self->gles = zgnr_gles_v32_create(self->display);
  if (self->gles == NULL) {
    zn_error("Failed to create zgnr_gles_v32");
    goto err_free;
  }

  self->new_rendering_unit_listener.notify =
      zn_appearance_handle_new_rendering_unit;
  wl_signal_add(&self->gles->events.new_rendering_unit,
      &self->new_rendering_unit_listener);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_appearance_destroy(struct zn_appearance* self)
{
  wl_list_remove(&self->new_rendering_unit_listener.link);
  zgnr_gles_v32_destroy(self->gles);
  free(self);
}
