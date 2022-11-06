#include "system.h"

#include <zen-common.h>
#include <zgnr/gl-buffer.h>
#include <zgnr/rendering-unit.h>

#include "scene/virtual-object/gl-buffer.h"
#include "scene/virtual-object/rendering-unit.h"
#include "zen/renderer/system.h"

static void
zna_system_handle_new_rendering_unit(struct wl_listener* listener, void* data)
{
  struct zna_system* self =
      zn_container_of(listener, self, new_rendering_unit_listener);
  struct zgnr_rendering_unit* unit = data;

  (void)zna_rendering_unit_create(unit, self);
}

static void
zna_system_handle_new_gl_buffer(struct wl_listener* listener, void* data)
{
  struct zna_system* self =
      zn_container_of(listener, self, new_gl_buffer_listener);
  struct zgnr_gl_buffer* gl_buffer = data;

  (void)zna_gl_buffer_create(gl_buffer, self);
}

struct zna_system*
zna_system_create(struct wl_display* display, struct znr_system* renderer)
{
  struct zna_system* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->display = display;
  self->renderer = renderer;

  self->gles = zgnr_gles_v32_create(self->display);
  if (self->gles == NULL) {
    zn_error("Failed to create zgnr_gles_v32");
    goto err_free;
  }

  self->new_rendering_unit_listener.notify =
      zna_system_handle_new_rendering_unit;
  wl_signal_add(&self->gles->events.new_rendering_unit,
      &self->new_rendering_unit_listener);

  self->new_gl_buffer_listener.notify = zna_system_handle_new_gl_buffer;
  wl_signal_add(
      &self->gles->events.new_gl_buffer, &self->new_gl_buffer_listener);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zna_system_destroy(struct zna_system* self)
{
  wl_list_remove(&self->new_rendering_unit_listener.link);
  wl_list_remove(&self->new_gl_buffer_listener.link);
  zgnr_gles_v32_destroy(self->gles);
  free(self);
}
