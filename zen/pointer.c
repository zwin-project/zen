#include "zen/pointer.h"

#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_pointer.h>

#include "zen-common.h"
#include "zen/cursor.h"
#include "zen/server.h"

struct zn_pointer*
zn_pointer_create(struct wlr_input_device* wlr_input_device)
{
  if (!zn_assert(wlr_input_device->type == WLR_INPUT_DEVICE_POINTER,
          "Wrong type - expect: %d, actual: %d", WLR_INPUT_DEVICE_POINTER,
          wlr_input_device->type)) {
    goto err;
  }

  struct zn_pointer* self;
  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->wlr_pointer = wlr_input_device->pointer;

  return self;

err:
  return NULL;
}

void
zn_pointer_destroy(struct zn_pointer* self)
{
  // wlr_pointer is destroyed by wlr_input_device
  free(self);
}
