#pragma once

#include <wayland-server-core.h>
#include <zgnr/gl-base-technique.h>
#include <zgnr/mem-storage.h>

#include "shader-inventory.h"
#include "system.h"
#include "zen/renderer/gl-base-technique.h"
#include "zen/renderer/gl-buffer.h"
#include "zen/renderer/gl-program.h"
#include "zen/renderer/gl-shader.h"
#include "zen/renderer/gl-vertex-array.h"
#include "zen/renderer/rendering-unit.h"
#include "zen/renderer/virtual-object.h"

struct zna_base_unit_vertex_attribute {
  uint32_t index;
  int32_t size;
  uint32_t type;
  bool normalized;
  int32_t stride;
  uint32_t offset;
};

struct zna_base_unit {
  struct zna_system* system;

  bool has_renderer_objects;
  struct znr_rendering_unit* rendering_unit;
  struct znr_gl_base_technique* technique;
  struct znr_gl_buffer* vertex_buffer;
  struct znr_gl_vertex_array* vertex_array;
  struct znr_gl_program* program;

  enum zna_shader_name vertex_shader;
  enum zna_shader_name fragment_shader;

  struct zgnr_mem_storage* vertex_buffer_storage;  // nullable

  struct wl_array vertex_attributes;  // struct zna_base_unit_vertex_attribute

  enum zgnr_gl_base_technique_draw_method draw_method;
  union zgnr_gl_base_technique_draw_args draw_args;
};

void zna_base_unit_setup_renderer_objects(struct zna_base_unit* self,
    struct znr_session* session, struct znr_virtual_object* virtual_object);

void zna_base_unit_teardown_renderer_objects(struct zna_base_unit* self);

struct zna_base_unit* zna_base_unit_create(struct zna_system* system,
    enum zna_shader_name vertex_shader, enum zna_shader_name fragment_shader,
    struct zgnr_mem_storage* vertex_buffer, struct wl_array* vertex_attributes,
    enum zgnr_gl_base_technique_draw_method draw_method,
    union zgnr_gl_base_technique_draw_args draw_args);

void zna_base_unit_destroy(struct zna_base_unit* self);
