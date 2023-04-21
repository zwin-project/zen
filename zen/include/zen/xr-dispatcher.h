#pragma once

#include <wayland-server-core.h>

#include "zen-common/util.h"

#ifdef __cplusplus
extern "C" {
#endif

struct zn_gl_base_technique;
struct zn_gl_buffer;
struct zn_gl_rendering_unit;
struct zn_virtual_object;
struct zn_xr_dispatcher;

struct zn_xr_dispatcher_interface {
  struct zn_virtual_object *(*get_new_virtual_object)(
      struct zn_xr_dispatcher *self);
  void (*destroy_virtual_object)(
      struct zn_xr_dispatcher *self, struct zn_virtual_object *virtual_object);

  struct zn_gl_rendering_unit *(*get_new_gl_rendering_unit)(
      struct zn_xr_dispatcher *self, struct zn_virtual_object *virtual_object);
  void (*destroy_gl_rendering_unit)(struct zn_xr_dispatcher *self,
      struct zn_gl_rendering_unit *gl_rendering_unit);

  struct zn_gl_base_technique *(*get_new_gl_base_technique)(
      struct zn_xr_dispatcher *self,
      struct zn_gl_rendering_unit *gl_rendering_unit);
  void (*destroy_gl_base_technique)(struct zn_xr_dispatcher *self,
      struct zn_gl_base_technique *gl_base_technique);

  struct zn_gl_buffer *(*get_new_gl_buffer)(struct zn_xr_dispatcher *self);
  void (*destroy_gl_buffer)(
      struct zn_xr_dispatcher *self, struct zn_gl_buffer *gl_buffer);
};

struct zn_xr_dispatcher {
  void *impl_data;
  const struct zn_xr_dispatcher_interface *impl;

  struct {
    struct wl_signal destroy;
  } events;
};

UNUSED static inline struct zn_virtual_object *
zn_xr_dispatcher_get_new_virtual_object(struct zn_xr_dispatcher *self)
{
  return self->impl->get_new_virtual_object(self);
}

UNUSED static inline void
zn_xr_dispatcher_destroy_virtual_object(
    struct zn_xr_dispatcher *self, struct zn_virtual_object *virtual_object)
{
  self->impl->destroy_virtual_object(self, virtual_object);
}

UNUSED static inline struct zn_gl_rendering_unit *
zn_xr_dispatcher_get_new_gl_rendering_unit(
    struct zn_xr_dispatcher *self, struct zn_virtual_object *virtual_object)
{
  return self->impl->get_new_gl_rendering_unit(self, virtual_object);
}

UNUSED static inline void
zn_xr_dispatcher_destroy_gl_rendering_unit(struct zn_xr_dispatcher *self,
    struct zn_gl_rendering_unit *gl_rendering_unit)
{
  self->impl->destroy_gl_rendering_unit(self, gl_rendering_unit);
}

UNUSED static inline struct zn_gl_base_technique *
zn_xr_dispatcher_get_new_gl_base_technique(
    struct zn_xr_dispatcher *self, struct zn_gl_rendering_unit *gl_rendering_unit)
{
  return self->impl->get_new_gl_base_technique(self, gl_rendering_unit);
}

UNUSED static inline void
zn_xr_dispatcher_destroy_gl_base_technique(
    struct zn_xr_dispatcher *self, struct zn_gl_base_technique *gl_base_technique)
{
  self->impl->destroy_gl_base_technique(self, gl_base_technique);
}

UNUSED static inline struct zn_gl_buffer *
zn_xr_dispatcher_get_new_gl_buffer(struct zn_xr_dispatcher *self)
{
  return self->impl->get_new_gl_buffer(self);
}

UNUSED static inline void
zn_xr_dispatcher_destroy_gl_buffer(
    struct zn_xr_dispatcher *self, struct zn_gl_buffer *gl_buffer)
{
  self->impl->destroy_gl_buffer(self, gl_buffer);
}

#ifdef __cplusplus
}
#endif
