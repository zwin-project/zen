#include "rendering-unit.h"

#include <zen-common.h>
#include <zigen-gles-v32-protocol.h>

#include "gl-base-technique.h"
#include "virtual-object.h"

static void zgnr_rendering_unit_destroy(struct zgnr_rendering_unit_impl* self);
static void zgnr_rendering_unit_inert(struct zgnr_rendering_unit_impl* self);

/**
 * @param technique is nullable
 */
void
zgnr_rendering_unit_set_current_technique(struct zgnr_rendering_unit_impl* self,
    struct zgnr_gl_base_technique_impl* technique)
{
  if (technique && self->base.current.type != ZGNR_TECHNIQUE_NONE) {
    wl_resource_post_error(technique->resource,
        ZGN_GL_BASE_TECHNIQUE_ERROR_TECHNIQUE,
        "rendering_unit must not have any other technique");
    return;
  }

  if (self->base.current.technique) {
    wl_list_remove(&self->current_technique_destroy_listener.link);
    wl_list_init(&self->current_technique_destroy_listener.link);
  }

  if (technique) {
    wl_signal_add(&technique->base.events.destroy,
        &self->current_technique_destroy_listener);
  }

  self->base.current.technique = &technique->base;
  self->base.current.type = ZGNR_TECHNIQUE_BASE;
}

static void
zgnr_rendering_unit_handle_destroy(struct wl_resource* resource)
{
  struct zgnr_rendering_unit_impl* self = wl_resource_get_user_data(resource);
  zgnr_rendering_unit_destroy(self);
}

static void
zgnr_rendering_unit_protocol_destroy(
    struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

/** Be careful, resource can be inert. */
static const struct zgn_rendering_unit_interface implementation = {
    .destroy = zgnr_rendering_unit_protocol_destroy,
};

static void
zgnr_rendering_unit_handle_current_technique_destroy(
    struct wl_listener* listener, void* data)
{
  UNUSED(data);

  struct zgnr_rendering_unit_impl* self =
      zn_container_of(listener, self, current_technique_destroy_listener);

  zgnr_rendering_unit_set_current_technique(self, NULL);
}

static void
zgnr_rendering_unit_handle_virtual_object_destroy(
    struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zgnr_rendering_unit_impl* self =
      zn_container_of(listener, self, virtual_object_destroy_listener);

  zgnr_rendering_unit_inert(self);
}

static void
zgnr_rendering_unit_handle_virtual_object_commit(
    struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zgnr_rendering_unit_impl* self =
      zn_container_of(listener, self, virtual_object_commit_listener);

  if (self->base.committed == false) {
    wl_list_insert(&self->base.virtual_object->current.rendering_unit_list,
        &self->base.link);
    self->base.committed = true;
  }

  wl_signal_emit(&self->events.on_commit, NULL);
}

static void
zgnr_rendering_unit_inert(struct zgnr_rendering_unit_impl* self)
{
  struct wl_resource* resource = self->resource;
  zgnr_rendering_unit_destroy(self);
  wl_resource_set_user_data(resource, NULL);
  wl_resource_set_destructor(resource, NULL);
}

struct zgnr_rendering_unit_impl*
zgnr_rendering_unit_create(struct wl_client* client, uint32_t id,
    struct zgnr_virtual_object_impl* virtual_object)
{
  struct zgnr_rendering_unit_impl* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->resource =
      wl_resource_create(client, &zgn_rendering_unit_interface, 1, id);
  if (self->resource == NULL) {
    zn_error("Failed to create a wl_resource");
    goto err_free;
  }

  wl_resource_set_implementation(self->resource, &implementation, self,
      zgnr_rendering_unit_handle_destroy);

  self->base.virtual_object = &virtual_object->base;

  wl_signal_init(&self->base.events.destroy);
  wl_signal_init(&self->events.on_commit);
  wl_list_init(&self->base.link);
  self->base.committed = false;
  self->base.current.technique = NULL;
  self->base.current.type = ZGNR_TECHNIQUE_NONE;

  self->virtual_object_destroy_listener.notify =
      zgnr_rendering_unit_handle_virtual_object_destroy;
  wl_signal_add(&virtual_object->base.events.destroy,
      &self->virtual_object_destroy_listener);

  self->virtual_object_commit_listener.notify =
      zgnr_rendering_unit_handle_virtual_object_commit;
  wl_signal_add(
      &virtual_object->events.on_commit, &self->virtual_object_commit_listener);

  self->current_technique_destroy_listener.notify =
      zgnr_rendering_unit_handle_current_technique_destroy;
  wl_list_init(&self->current_technique_destroy_listener.link);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zgnr_rendering_unit_destroy(struct zgnr_rendering_unit_impl* self)
{
  wl_signal_emit(&self->base.events.destroy, NULL);

  wl_list_remove(&self->base.link);
  wl_list_remove(&self->current_technique_destroy_listener.link);
  wl_list_remove(&self->virtual_object_destroy_listener.link);
  wl_list_remove(&self->base.events.destroy.listener_list);
  wl_list_remove(&self->virtual_object_commit_listener.link);
  wl_list_remove(&self->events.on_commit.listener_list);
  free(self);
}
