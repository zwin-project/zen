#include "zen/xdg-popup.h"

#include "zen-common.h"
#include "zen/scene/view-child.h"

struct zn_xdg_popup*
zn_xdg_popup_create(struct wlr_xdg_popup* wlr_xdg_popup)
{
  struct zn_xdg_popup* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    wl_resource_post_no_memory(wlr_xdg_popup->resource);
    goto err;
  }

  view_child_init();

err:
  return NULL;
}
