#include "zen/input-manager.h"

#include <wayland-server.h>
#include <wlr/types/wlr_input_device.h>

#include "zen-common.h"
#include "zen/input-device.h"
#include "zen/seat.h"

struct zn_input_manager {
  struct zn_seat* seat;
};

void
zn_input_manager_handle_new_wlr_input(
    struct zn_input_manager* self, struct wlr_input_device* wlr_input)
{
  struct zn_input_device* input_device =
      zn_input_device_create(self->seat, wlr_input);
  if (input_device == NULL) {
    zn_error("Failed to create zn_input_device");
    return;
  }

  // TODO: add multi seat support
}

struct zn_input_manager*
zn_input_manager_create(struct wl_display* display)
{
  struct zn_input_manager* self;
  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->seat = zn_seat_create(display, ZEN_DEFAULT_SEAT);
  if (self->seat == NULL) {
    zn_error("Failed to create zn_seat");
    goto err_free;
  }

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_input_manager_destroy(struct zn_input_manager* self)
{
  zn_seat_destroy(self->seat);
  free(self);
}
