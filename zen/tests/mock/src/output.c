#include "mock/output.h"

#include <assert.h>
#include <wlr/util/box.h>

#include "screen.h"
#include "zen-common/util.h"
#include "zen/snode.h"

static void
zn_mock_output_damage(void *impl_data, struct wlr_fbox *damage_box UNUSED)
{
  struct zn_mock_output *self = impl_data;
  pixman_region32_union_rect(&self->damage, &self->damage, (int)damage_box->x,
      (int)damage_box->y, (int)damage_box->width, (int)damage_box->height);
}

static struct zn_snode *
zn_mock_output_get_layer(void *impl_data, enum zwlr_layer_shell_v1_layer layer)
{
  struct zn_mock_output *self = impl_data;
  return self->layers[layer];
}

static const struct zn_screen_interface screen_implementation = {
    .damage = zn_mock_output_damage,
    .get_layer = zn_mock_output_get_layer,
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

void
zn_mock_output_resize(struct zn_mock_output *self, float width, float height)
{
  zn_screen_notify_resize(self->screen, (vec2){width, height});
}

struct zn_mock_output *
zn_mock_output_create(float width, float height)
{
  struct zn_mock_output *self = zalloc(sizeof *self);
  assert(self);

  for (int i = 0; i < 4; i++) {
    self->layers[i] = zn_snode_create(self, &zn_snode_noop_implementation);
  }

  self->screen = zn_screen_create(self, &screen_implementation);
  assert(self->screen);

  pixman_region32_init(&self->damage);

  zn_screen_notify_resize(self->screen, (vec2){width, height});

  return self;
}

void
zn_mock_output_destroy(struct zn_mock_output *self)
{
  for (int i = 0; i < 4; i++) {
    zn_snode_destroy(self->layers[i]);
  }
  pixman_region32_fini(&self->damage);
  zn_screen_destroy(self->screen);
  free(self);
}
