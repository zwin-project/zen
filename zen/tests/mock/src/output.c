#include "mock/output.h"

#include <assert.h>
#include <wlr/util/box.h>

#include "zen-common/util.h"
#include "zen/screen.h"

static void
zn_mock_output_damage(void *impl_data, struct wlr_fbox *damage_box UNUSED)
{
  struct zn_mock_output *self = impl_data;
  pixman_region32_union_rect(&self->damage, &self->damage, (int)damage_box->x,
      (int)damage_box->y, (int)damage_box->width, (int)damage_box->height);
}

static const struct zn_screen_interface screen_implementation = {
    .damage = zn_mock_output_damage,
};

bool
zn_mock_output_damage_contains(struct zn_mock_output *self, int x, int y)
{
  return pixman_region32_contains_point(&self->damage, x, y, NULL);
}

void
zn_mock_output_damage_clear(struct zn_mock_output *self)
{
  pixman_region32_clear(&self->damage);
}

struct zn_mock_output *
zn_mock_output_create(void)
{
  struct zn_mock_output *self = zalloc(sizeof *self);
  assert(self);

  self->screen = zn_screen_create(self, &screen_implementation);
  assert(self->screen);

  pixman_region32_init(&self->damage);

  return self;
}

void
zn_mock_output_destroy(struct zn_mock_output *self)
{
  pixman_region32_fini(&self->damage);
  zn_screen_destroy(self->screen);
  free(self);
}
