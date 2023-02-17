#include "mock/backend.h"

#include <assert.h>
#include <wlr/render/wlr_texture.h>

#include "zen-common/signal.h"
#include "zen-common/util.h"

static void zn_mock_backend_destroy(struct zn_mock_backend *self);

static struct wlr_texture *
zn_mock_backend_create_wlr_texture_from_pixels(struct zn_backend *base UNUSED,
    uint32_t format UNUSED, uint32_t stride UNUSED, uint32_t width,
    uint32_t height, const void *data UNUSED)
{
  struct wlr_texture *texture = zalloc(sizeof *texture);
  texture->width = width;
  texture->height = height;
  return texture;
}

static bool
zn_mock_backend_start(struct zn_backend *base UNUSED)
{
  return true;
}

static void
zn_mock_backend_handle_destroy(struct zn_backend *base)
{
  struct zn_mock_backend *self = zn_container_of(base, self, base);
  zn_mock_backend_destroy(self);
}

static const struct zn_backend_interface implementation = {
    .create_wlr_texture_from_pixels =
        zn_mock_backend_create_wlr_texture_from_pixels,
    .start = zn_mock_backend_start,
    .destroy = zn_mock_backend_handle_destroy,
};

struct zn_mock_backend *
zn_mock_backend_create(void)
{
  struct zn_mock_backend *self = zalloc(sizeof *self);
  assert(self);

  self->base.impl = &implementation;
  wl_signal_init(&self->base.events.new_screen);
  wl_signal_init(&self->base.events.destroy);

  return self;
}

static void
zn_mock_backend_destroy(struct zn_mock_backend *self)
{
  zn_signal_emit_mutable(&self->base.events.destroy, NULL);

  wl_list_remove(&self->base.events.destroy.listener_list);
  wl_list_remove(&self->base.events.new_screen.listener_list);
  free(self);
}
