#include "zen-desktop/ui/decoration.h"

#include <cglm/vec2.h>
#include <zen-common/log.h>
#include <zen-common/util.h>

#include "zen-desktop/ui/header-bar.h"
#include "zen/snode.h"

#define HEADER_HEIGHT 30

void
zn_ui_decoration_set_content_size(struct zn_ui_decoration *self, vec2 size)
{
  zn_ui_header_bar_set_size(self->header_bar, (vec2){size[0], HEADER_HEIGHT});

  glm_vec2_copy(size, self->content_size);
  glm_vec2_copy((vec2){0, HEADER_HEIGHT}, self->content_offset);
}

struct zn_ui_decoration *
zn_ui_decoration_create(void)
{
  struct zn_ui_decoration *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  glm_vec2_copy(GLM_VEC2_ZERO, self->content_size);
  glm_vec2_copy(GLM_VEC2_ZERO, self->content_offset);

  self->snode = zn_snode_create(self, &zn_snode_noop_implementation);
  if (self->snode == NULL) {
    zn_error("Failed to create a snode");
    goto err_free;
  }

  self->header_bar = zn_ui_header_bar_create();
  if (self->header_bar == NULL) {
    zn_error("Failed to create a zn_ui_header_bar");
    goto err_snode;
  }

  zn_snode_set_position(self->header_bar->snode, self->snode, GLM_VEC2_ZERO);

  return self;

err_snode:
  zn_snode_destroy(self->snode);

err_free:
  free(self);

err:
  return NULL;
}

void
zn_ui_decoration_destroy(struct zn_ui_decoration *self)
{
  zn_ui_header_bar_destroy(self->header_bar);
  zn_snode_destroy(self->snode);
  free(self);
}
