#include "shader-inventory.h"

#include <GLES3/gl32.h>
#include <zen-common.h>

#include "board.frag.h"
#include "board.vert.h"
#include "bounded-nameplate.frag.h"
#include "bounded-nameplate.vert.h"
#include "color.frag.h"
#include "ray.frag.h"
#include "ray.vert.h"
#include "system.h"
#include "view.frag.h"
#include "view.vert.h"

struct shader_info {
  const char *source;
  size_t length;
  GLenum type;
};

static const struct shader_info shader_info[ZNA_SHADER_COUNT] = {
    [ZNA_SHADER_BOARD_VERTEX] =
        {
            .source = board_vert_source,
            .length = sizeof(board_vert_source),
            .type = GL_VERTEX_SHADER,
        },
    [ZNA_SHADER_BOARD_FRAGMENT] =
        {
            .source = board_frag_source,
            .length = sizeof(board_frag_source),
            .type = GL_FRAGMENT_SHADER,
        },
    [ZNA_SHADER_BOUNDED_NAMEPLATE_VERTEX] =
        {
            .source = bounded_nameplate_vert_source,
            .length = sizeof(bounded_nameplate_vert_source),
            .type = GL_VERTEX_SHADER,
        },
    [ZNA_SHADER_BOUNDED_NAMEPLATE_FRAGMENT] =
        {
            .source = bounded_nameplate_frag_source,
            .length = sizeof(bounded_nameplate_frag_source),
            .type = GL_FRAGMENT_SHADER,
        },
    [ZNA_SHADER_COLOR_FRAGMENT] =
        {
            .source = color_frag_source,
            .length = sizeof(color_frag_source),
            .type = GL_FRAGMENT_SHADER,
        },
    [ZNA_SHADER_RAY_VERTEX] =
        {
            .source = ray_vert_source,
            .length = sizeof(ray_vert_source),
            .type = GL_VERTEX_SHADER,
        },
    [ZNA_SHADER_RAY_FRAGMENT] =
        {
            .source = ray_frag_source,
            .length = sizeof(ray_frag_source),
            .type = GL_FRAGMENT_SHADER,
        },
    [ZNA_SHADER_VIEW_VERTEX] =
        {
            .source = view_vert_source,
            .length = sizeof(view_vert_source),
            .type = GL_VERTEX_SHADER,
        },
    [ZNA_SHADER_VIEW_FRAGMENT] =
        {
            .source = view_frag_source,
            .length = sizeof(view_frag_source),
            .type = GL_FRAGMENT_SHADER,
        },
};

struct zna_shader_inventory {
  struct znr_gl_shader *shaders[ZNA_SHADER_COUNT];
  struct zna_system *system;

  struct wl_listener session_destroyed_listener;
};

static void
zna_shader_inventory_handle_session_destroyed(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zna_shader_inventory *self =
      zn_container_of(listener, self, session_destroyed_listener);

  for (int i = 0; i < ZNA_SHADER_COUNT; i++) {
    if (self->shaders[i]) {
      znr_gl_shader_destroy(self->shaders[i]);
      self->shaders[i] = NULL;
    }
  }
}

struct znr_gl_shader *
zna_shader_inventory_get(struct zna_shader_inventory *self,
    enum zna_shader_name name, struct znr_dispatcher *dispatcher)
{
  struct znr_session *session = self->system->current_session;

  if (!session || name >= ZNA_SHADER_COUNT) return NULL;

  if (self->shaders[name]) {
    return self->shaders[name];
  }

  const struct shader_info *info = &shader_info[name];

  self->shaders[name] =
      znr_gl_shader_create(dispatcher, info->source, info->length, info->type);

  return self->shaders[name];
}

struct zna_shader_inventory *
zna_shader_inventory_create(struct zna_system *system)
{
  struct zna_shader_inventory *self;

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
zna_shader_inventory_destroy(struct zna_shader_inventory *self)
{
  for (int i = 0; i < ZNA_SHADER_COUNT; i++) {
    if (self->shaders[i]) {
      znr_gl_shader_destroy(self->shaders[i]);
    }
  }
  wl_list_remove(&self->session_destroyed_listener.link);
  free(self);
}
