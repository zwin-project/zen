#include "program-shader.h"

#include <zen-common.h>

#include "gl-program.h"
#include "gl-shader.h"

static void
zgnr_program_shader_handle_shader_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zgnr_program_shader_impl *self =
      zn_container_of(listener, self, shader_destroy_listener);

  zgnr_program_shader_destroy(self);
}

struct zgnr_program_shader_impl *
zgnr_program_shader_create(
    struct zgnr_gl_program_impl *program, struct zgnr_gl_shader_impl *shader)
{
  struct zgnr_program_shader_impl *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->base.shader = &shader->base;
  self->program = program;
  wl_list_init(&self->base.link);

  self->shader_destroy_listener.notify =
      zgnr_program_shader_handle_shader_destroy;
  wl_signal_add(&shader->base.events.destroy, &self->shader_destroy_listener);

  return self;

err:
  return NULL;
}

void
zgnr_program_shader_destroy(struct zgnr_program_shader_impl *self)
{
  wl_list_remove(&self->base.link);
  wl_list_remove(&self->shader_destroy_listener.link);
  free(self);
}
