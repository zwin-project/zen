#ifndef ZEN_CURSOR_GRUB_H
#define ZEN_CURSOR_GRUB_H

#include <wayland-server.h>
#include <wlr/interfaces/wlr_pointer.h>
#include <wlr/types/wlr_cursor.h>

#include "zen/cursor.h"

struct zn_cursor_grab;

struct zn_cursor_grab_interface {
  void (*motion)(
      struct zn_cursor_grab* grab, struct wlr_event_pointer_motion* event);
  void (*button)(
      struct zn_cursor_grab* grab, struct wlr_event_pointer_button* event);
  void (*axis)(
      struct zn_cursor_grab* grab, struct wlr_event_pointer_axis* event);
  void (*frame)(struct zn_cursor_grab* grab);
};

struct zn_cursor_grab {
  const struct zn_cursor_grab_interface* interface;
  struct zn_cursor* cursor;
};

#endif  // ZEN_CURSOR_GRUB_H
