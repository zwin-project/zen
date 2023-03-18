#include "zen-desktop/theme.h"

#include "zen-common/log.h"
#include "zen-common/util.h"

struct zn_theme *
zn_theme_create(void)
{
  struct zn_theme *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  zn_color_init(&self->color.header_bar.active, 0x040d29ff);
  zn_color_init(&self->color.header_bar.inactive, 0x596383ff);
  zn_color_init(&self->color.header_bar.active_reflection, 0x111f4dff);
  zn_color_init(&self->color.header_bar.inactive_reflection, 0x878ea5ff);

  self->radius.header_bar.corner = 8.F;

  self->size.header_bar.height = 30.F;

  zn_drop_shadow_init(
      &self->shadow.view, 35.F, 0.45F, self->radius.header_bar.corner);

  return self;

err:
  return NULL;
}

void
zn_theme_destroy(struct zn_theme *self)
{
  zn_color_fini(&self->color.header_bar.active);
  zn_color_fini(&self->color.header_bar.inactive);
  zn_color_fini(&self->color.header_bar.active_reflection);
  zn_color_fini(&self->color.header_bar.inactive_reflection);

  zn_drop_shadow_fini(&self->shadow.view);

  free(self);
}
