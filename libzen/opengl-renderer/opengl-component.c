#include "opengl-component.h"

#include <wayland-server.h>
#include <zigen-opengl-server-protocol.h>

WL_EXPORT
void zen_opengl_component_destroy(struct zen_opengl_component *component);

static void
zen_opengl_component_handle_destroy(struct wl_resource *resource)
{
  struct zen_opengl_component *component;

  component = wl_resource_get_user_data(resource);

  zen_opengl_component_destroy(component);
}

static void
zen_opengl_component_protocol_destroy(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void
zen_opengl_component_protocol_attach_vertex_buffer(struct wl_client *client,
    struct wl_resource *resource, struct wl_resource *vertex_buffer)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(vertex_buffer);
}

static void
zen_opengl_component_protocol_attach_shader_program(struct wl_client *client,
    struct wl_resource *resource, struct wl_resource *shader_program)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(shader_program);
}

static void
zen_opengl_component_protocol_attach_texture(struct wl_client *client,
    struct wl_resource *resource, struct wl_resource *texture)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(texture);
}

static void
zen_opengl_component_protocol_add_vertex_attribute(struct wl_client *client,
    struct wl_resource *resource, uint32_t index, uint32_t size, uint32_t type,
    uint32_t normalized, uint32_t stride, uint32_t pointer)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(index);
  UNUSED(size);
  UNUSED(type);
  UNUSED(normalized);
  UNUSED(stride);
  UNUSED(pointer);
}

static void
zen_opengl_component_protocol_clear_vertex_attribute(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  UNUSED(resource);
}

static void
zen_opengl_component_protocol_set_topology(
    struct wl_client *client, struct wl_resource *resource, uint32_t topology)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(topology);
}

static void
zen_opengl_component_protocol_set_min(
    struct wl_client *client, struct wl_resource *resource, uint32_t min)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(min);
}

static void
zen_opengl_component_protocol_set_count(
    struct wl_client *client, struct wl_resource *resource, uint32_t count)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(count);
}

static const struct zgn_opengl_component_interface opengl_component_interface =
    {
        .destroy = zen_opengl_component_protocol_destroy,
        .attach_vertex_buffer =
            zen_opengl_component_protocol_attach_vertex_buffer,
        .attach_shader_program =
            zen_opengl_component_protocol_attach_shader_program,
        .attach_texture = zen_opengl_component_protocol_attach_texture,
        .add_vertex_attribute =
            zen_opengl_component_protocol_add_vertex_attribute,
        .clear_vertex_attribute =
            zen_opengl_component_protocol_clear_vertex_attribute,
        .set_topology = zen_opengl_component_protocol_set_topology,
        .set_min = zen_opengl_component_protocol_set_min,
        .set_count = zen_opengl_component_protocol_set_count,
};

WL_EXPORT struct zen_opengl_component *
zen_opengl_component_create(struct wl_client *client, uint32_t id,
    struct zen_virtual_object *virtual_object)
{
  struct zen_opengl_component *component;
  struct wl_resource *resource;

  component = zalloc(sizeof *component);
  if (component == NULL) {
    zen_log("opengl component: failed to allocate memory\n");
    wl_client_post_no_memory(client);
    goto err;
  }

  resource = wl_resource_create(client, &zgn_opengl_component_interface, 1, id);
  if (resource == NULL) {
    zen_log("opengl component: failed to create a resource\n");
    wl_client_post_no_memory(client);
    goto err_resource;
  }

  wl_resource_set_implementation(resource, &opengl_component_interface,
      component, zen_opengl_component_handle_destroy);

  component->virtual_object = virtual_object;

  return component;

err_resource:
  free(component);

err:
  return NULL;
}

WL_EXPORT
void
zen_opengl_component_destroy(struct zen_opengl_component *component)
{
  free(component);
}
