#include "shader-inventory.h"

#include <GLES3/gl32.h>
#include <zen-common.h>

#include "color.frag.h"
#include "default.vert.h"
#include "system.h"

struct zna_shader_inventory {
  struct znr_gl_shader* shaders[ZNA_SHADER_COUNT];
  struct zna_system* system;

  struct wl_listener session_destroyed_listener;
};

static void
zna_shader_inventory_handle_session_destroyed(
    struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zna_shader_inventory* self =
      zn_container_of(listener, self, session_destroyed_listener);

  for (int i = 0; i < ZNA_SHADER_COUNT; i++) {
    if (self->shaders[i]) {
      znr_gl_shader_destroy(self->shaders[i]);
      self->shaders[i] = NULL;
    }
  }
}

struct znr_gl_shader*
zna_shader_inventory_get(
    struct zna_shader_inventory* self, enum zna_shader_name name)
{
  struct znr_session* session = self->system->current_session;
  const char* shader_source;
  uint32_t type;
  size_t length;

  if (!session || name >= ZNA_SHADER_COUNT) return NULL;

  if (self->shaders[name]) {
    return self->shaders[name];
  }

  switch (name) {
    case ZNA_SHADER_DEFAULT_VERTEX:
      shader_source = default_vert_source;
      length = sizeof(default_vert_source);
      type = GL_VERTEX_SHADER;
      break;

    case ZNA_SHADER_COLOR_FRAGMENT:
      shader_source = color_frag_source;
      length = sizeof(color_frag_source);
      type = GL_FRAGMENT_SHADER;
      break;

    default:
      zn_assert(false, "Unknown shader name");
      return NULL;
  }

  self->shaders[name] =
      znr_gl_shader_create(session, shader_source, length, type);

  return self->shaders[name];
}

struct zna_shader_inventory*
zna_shader_inventory_create(struct zna_system* system)
{
  struct zna_shader_inventory* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    goto err;
  }

  self->system = system;

  self->session_destroyed_listener.notify =
      zna_shader_inventory_handle_session_destroyed;
  wl_signal_add(&system->events.current_session_destroyed,
      &self->session_destroyed_listener);

  return self;

err:
  return NULL;
}

void
zna_shader_inventory_destroy(struct zna_shader_inventory* self)
{
  for (int i = 0; i < ZNA_SHADER_COUNT; i++) {
    if (self->shaders[i]) {
      znr_gl_shader_destroy(self->shaders[i]);
    }
  }
  wl_list_remove(&self->session_destroyed_listener.link);
  free(self);
}
