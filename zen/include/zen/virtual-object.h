#pragma once

#include <wayland-server-core.h>

#include "zen-common/util.h"

#ifdef __cplusplus
extern "C" {
#endif

struct zn_bounded;
struct zn_gl_virtual_object;
struct zn_inode;

enum zn_virtual_object_role {
  ZN_VIRTUAL_OBJECT_ROLE_NONE = 0,
  ZN_VIRTUAL_OBJECT_ROLE_BOUNDED,
};

struct zn_virtual_object {
  struct zn_inode *inode;  // @nonnull, @owning

  struct zn_gl_virtual_object *gl_virtual_object;  // @nullable, @ref

  union {
    void *role_object;           // @nullable, @ref
    struct zn_bounded *bounded;  // when role == BOUNDED
  };
  enum zn_virtual_object_role role;

  struct wl_listener role_object_destroy_listener;

  struct {
    struct wl_signal destroy;  // (NULL)
    struct wl_signal commit;   // (NULL)
  } events;
};

void zn_virtual_object_commit(struct zn_virtual_object *self);

void zn_virtual_object_change_visibility(
    struct zn_virtual_object *self, bool visible);

/// @return true if successful, false otherwise
bool zn_virtual_object_set_role(struct zn_virtual_object *self,
    enum zn_virtual_object_role role, void *role_object);

struct zn_virtual_object *zn_virtual_object_create(void);

void zn_virtual_object_destroy(struct zn_virtual_object *self);

#ifdef __cplusplus
}
#endif
