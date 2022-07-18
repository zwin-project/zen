#include "seat.h"

#include <wlr/types/wlr_seat.h>

#include "zen-common.h"

struct zn_seat {
  struct wlr_seat* wlr_seat;
  // struct wl_listener destroy;
};

struct zn_seat*
zn_seat_create(struct wl_display* display, const char* seat_name)
{
  struct zn_seat* self;
  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->wlr_seat = wlr_seat_create(display, seat_name);
  if (self->wlr_seat == NULL) {
    zn_error("Failed to create wlr_seat");
    goto err_free;
  }

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_seat_destroy(struct zn_seat* self)
{
  wlr_seat_destroy(self->wlr_seat);
  free(self);
}
