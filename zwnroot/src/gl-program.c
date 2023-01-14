#include "gl-program.h"

#include <zen-common.h>
#include <zwin-gles-v32-protocol.h>

#include "program-shader.h"

static void zwnr_gl_program_destroy(struct zwnr_gl_program_impl *self);

static void
zwnr_gl_program_handle_destroy(struct wl_resource *resource)
{
  struct zwnr_gl_program_impl *self = wl_resource_get_user_data(resource);

  zwnr_gl_program_destroy(self);
}

static void
zwnr_gl_program_protocol_destroy(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void
zwnr_gl_program_protocol_attach_shader(struct wl_client *client,
    struct wl_resource *resource, struct wl_resource *shader_resource)
{
  struct zwnr_gl_program_impl *self = wl_resource_get_user_data(resource);
  struct zwnr_gl_shader_impl *shader =
      wl_resource_get_user_data(shader_resource);
  struct zwnr_program_shader_impl *program_shader;

  program_shader = zwnr_program_shader_create(self, shader);
  if (program_shader == NULL) {
    zn_error("Failed to create zwnr_program_shader");
    wl_client_post_no_memory(client);
    return;
  }

  wl_list_insert(
      &self->pending.program_shader_list, &program_shader->base.link);
  self->pending.damaged = true;
}

static void
zwnr_gl_program_protocol_link(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  struct zwnr_gl_program_impl *self = wl_resource_get_user_data(resource);

  self->pending.should_link = true;
  self->pending.damaged = true;
}

static const struct zwn_gl_program_interface implementation = {
    .destroy = zwnr_gl_program_protocol_destroy,
    .attach_shader = zwnr_gl_program_protocol_attach_shader,
    .link = zwnr_gl_program_protocol_link,
};

void
zwnr_gl_program_commit(struct zwnr_gl_program_impl *self)
{
  if (self->base.current.linked && self->pending.should_link) {
    wl_resource_post_error(self->resource, ZWN_GL_PROGRAM_ERROR_RELINK,
        "gl_program has already linked");
    return;
  }

  if (self->pending.damaged == false) return;

  wl_list_insert_list(&self->base.current.program_shader_list,
      &self->pending.program_shader_list);
  wl_list_init(&self->pending.program_shader_list);

  if (self->pending.should_link) {
    self->base.current.should_link = true;
    self->base.current.linked = true;
  }

  self->pending.should_link = false;
  self->pending.damaged = false;
}

struct zwnr_gl_program_impl *
zwnr_gl_program_create(struct wl_client *client, uint32_t id)
{
  struct zwnr_gl_program_impl *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->resource = wl_resource_create(client, &zwn_gl_program_interface, 1, id);
  if (self->resource == NULL) {
    zn_error("Failed to creat a wl_resource");
    goto err_free;
  }

  self->pending.should_link = false;
  wl_list_init(&self->pending.program_shader_list);
  self->base.current.linked = false;
  self->base.current.should_link = false;
  wl_list_init(&self->base.current.program_shader_list);

  wl_resource_set_implementation(
      self->resource, &implementation, self, zwnr_gl_program_handle_destroy);

  wl_signal_init(&self->base.events.destroy);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zwnr_gl_program_destroy(struct zwnr_gl_program_impl *self)
{
  wl_signal_emit(&self->base.events.destroy, NULL);

  struct zwnr_program_shader_impl *program_shader, *tmp;

  wl_list_for_each_safe (
      program_shader, tmp, &self->pending.program_shader_list, base.link) {
    zwnr_program_shader_destroy(program_shader);
  }

  wl_list_for_each_safe (
      program_shader, tmp, &self->base.current.program_shader_list, base.link) {
    zwnr_program_shader_destroy(program_shader);
  }

  wl_list_remove(&self->base.events.destroy.listener_list);
  free(self);
}
