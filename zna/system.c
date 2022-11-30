#include "system.h"

#include <zen-common.h>
#include <zgnr/gl-buffer.h>
#include <zgnr/gl-shader.h>
#include <zgnr/rendering-unit.h>

#include "shader-inventory.h"
#include "virtual-object/gl-base-technique.h"
#include "virtual-object/gl-buffer.h"
#include "virtual-object/gl-program.h"
#include "virtual-object/gl-shader.h"
#include "virtual-object/gl-texture.h"
#include "virtual-object/gl-vertex-array.h"
#include "virtual-object/rendering-unit.h"
#include "zen/server.h"

void
zna_system_set_current_session(
    struct zna_system* self, struct znr_session* session)
{
  struct zn_server* server = zn_server_get_singleton();

  if (self->current_session) {
    wl_list_remove(&self->current_session_disconnected_listener.link);
    wl_list_init(&self->current_session_disconnected_listener.link);
    wl_list_remove(&self->current_session_frame_listener.link);
    wl_list_init(&self->current_session_frame_listener.link);
    znr_session_destroy(self->current_session);
    self->current_session = NULL;

    zn_debug("The current session was destroyed");
    wl_signal_emit(&self->events.current_session_destroyed, NULL);
  }

  if (session) {
    self->current_session = session;
    wl_signal_add(&session->events.disconnected,
        &self->current_session_disconnected_listener);
    wl_signal_add(
        &session->events.frame, &self->current_session_frame_listener);

    zn_debug("The current session is newly created");
    wl_signal_emit(&self->events.current_session_created, NULL);

    zn_server_change_display_system(server, ZEN_DISPLAY_SYSTEM_TYPE_IMMERSIVE);
  } else {
    zn_server_change_display_system(server, ZEN_DISPLAY_SYSTEM_TYPE_SCREEN);
  }
}

static void
zna_system_handle_current_session_frame(
    struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zna_system* self =
      zn_container_of(listener, self, current_session_frame_listener);

  wl_signal_emit(&self->events.current_session_frame, NULL);
}

static void
zna_system_handle_current_session_disconnected(
    struct wl_listener* listener, void* data)
{
  struct zna_system* self =
      zn_container_of(listener, self, current_session_disconnected_listener);
  UNUSED(data);
  zna_system_set_current_session(self, NULL);
}

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

static void
zna_system_handle_new_gl_base_technique(
    struct wl_listener* listener, void* data)
{
  struct zna_system* self =
      zn_container_of(listener, self, new_gl_base_technique_listener);
  struct zgnr_gl_base_technique* gl_base_technique = data;

  (void)zna_gl_base_technique_create(gl_base_technique, self);
}

static void
zna_system_handle_new_gl_vertex_array(struct wl_listener* listener, void* data)
{
  struct zna_system* self =
      zn_container_of(listener, self, new_gl_vertex_array_listener);
  struct zgnr_gl_vertex_array* gl_vertex_array = data;

  (void)zna_gl_vertex_array_create(gl_vertex_array, self);
}

static void
zna_system_handle_new_gl_shader(struct wl_listener* listener, void* data)
{
  struct zna_system* self =
      zn_container_of(listener, self, new_gl_shader_listener);

  struct zgnr_gl_shader* shader = data;

  (void)zna_gl_shader_create(shader, self);
}

static void
zna_system_handle_new_gl_texture(struct wl_listener* listener, void* data)
{
  struct zna_system* self =
      zn_container_of(listener, self, new_gl_texture_listener);

  struct zgnr_gl_texture* texture = data;

  (void)zna_gl_texture_create(texture, self);
}

static void
zna_system_handle_new_gl_program(struct wl_listener* listener, void* data)
{
  struct zna_system* self =
      zn_container_of(listener, self, new_gl_program_listener);

  struct zgnr_gl_program* program = data;

  (void)zna_gl_program_create(program, self);
}

struct zna_system*
zna_system_create(struct wl_display* display)
{
  struct zna_system* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->display = display;
  self->current_session = NULL;

  self->gles = zgnr_gles_v32_create(self->display);
  if (self->gles == NULL) {
    zn_error("Failed to create zgnr_gles_v32");
    goto err_free;
  }

  wl_signal_init(&self->events.current_session_created);
  wl_signal_init(&self->events.current_session_destroyed);
  wl_signal_init(&self->events.current_session_frame);

  self->new_rendering_unit_listener.notify =
      zna_system_handle_new_rendering_unit;
  wl_signal_add(&self->gles->events.new_rendering_unit,
      &self->new_rendering_unit_listener);

  self->new_gl_buffer_listener.notify = zna_system_handle_new_gl_buffer;
  wl_signal_add(
      &self->gles->events.new_gl_buffer, &self->new_gl_buffer_listener);

  self->new_gl_base_technique_listener.notify =
      zna_system_handle_new_gl_base_technique;
  wl_signal_add(&self->gles->events.new_gl_base_technique,
      &self->new_gl_base_technique_listener);

  self->new_gl_vertex_array_listener.notify =
      zna_system_handle_new_gl_vertex_array;
  wl_signal_add(&self->gles->events.new_gl_vertex_array,
      &self->new_gl_vertex_array_listener);

  self->new_gl_shader_listener.notify = zna_system_handle_new_gl_shader;
  wl_signal_add(
      &self->gles->events.new_gl_shader, &self->new_gl_shader_listener);

  self->new_gl_program_listener.notify = zna_system_handle_new_gl_program;
  wl_signal_add(
      &self->gles->events.new_gl_program, &self->new_gl_program_listener);

  self->new_gl_texture_listener.notify = zna_system_handle_new_gl_texture;
  wl_signal_add(
      &self->gles->events.new_gl_texture, &self->new_gl_texture_listener);

  self->current_session_disconnected_listener.notify =
      zna_system_handle_current_session_disconnected;
  wl_list_init(&self->current_session_disconnected_listener.link);

  self->current_session_frame_listener.notify =
      zna_system_handle_current_session_frame;
  wl_list_init(&self->current_session_frame_listener.link);

  self->shader_inventory = zna_shader_inventory_create(self);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zna_system_destroy(struct zna_system* self)
{
  if (self->current_session) znr_session_destroy(self->current_session);

  zna_shader_inventory_destroy(self->shader_inventory);

  wl_list_remove(&self->events.current_session_created.listener_list);
  wl_list_remove(&self->events.current_session_destroyed.listener_list);
  wl_list_remove(&self->events.current_session_frame.listener_list);
  wl_list_remove(&self->new_rendering_unit_listener.link);
  wl_list_remove(&self->new_gl_buffer_listener.link);
  wl_list_remove(&self->new_gl_base_technique_listener.link);
  wl_list_remove(&self->new_gl_vertex_array_listener.link);
  wl_list_remove(&self->new_gl_shader_listener.link);
  wl_list_remove(&self->new_gl_program_listener.link);
  wl_list_remove(&self->new_gl_texture_listener.link);
  wl_list_remove(&self->current_session_disconnected_listener.link);
  wl_list_remove(&self->current_session_frame_listener.link);
  zgnr_gles_v32_destroy(self->gles);
  free(self);
}
