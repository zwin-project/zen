#ifndef ZEN_POINTER_GRUB_H
#define ZEN_POINTER_GRUB_H

#include <wayland-server.h>
#include <wlr/types/wlr_pointer.h>

#include "zen/input/pointer.h"

struct zn_pointer_grab;

struct zn_pointer_grab_interface {
  void (*motion)(
      struct zn_pointer_grab* grab, struct wlr_event_pointer_motion* event);
  void (*button)(
      struct zn_pointer_grab* grab, struct wlr_event_pointer_button* event);
};

struct zn_pointer_grab {
  const struct zn_pointer_grab_interface* interface;
  struct zn_pointer* pointer;
};

#endif  // ZEN_POINTER_GRUB_H
