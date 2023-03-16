#pragma once

///
/// "edge box"   : outer rectangle
/// "content box": inner rectangle:
/// "edge area"  : the area between "edge box" and "content box"
///
/// The content box includes header bar.
/// _________________________________
/// | +---------------------------+ |
/// | |                           | |
/// | |                           | |
/// | |                           | |
/// | |                           | |
/// | |                           | |
/// | |                           | |
/// | |___________________________| |
/// +-------------------------------+
///

#include <cglm/types.h>
#include <wayland-server-core.h>

struct zn_ui_decoration_edge_pressed_event {
  uint32_t edges;  // a bitfield of enum wlr_edges
};

struct zn_ui_decoration_edge_hover_event {
  uint32_t edges;  // a bitfield of enum wlr_edges
};

struct zn_ui_decoration_edge {
  /// The origin of snode is the left top corner of "content" box.
  struct zn_snode *snode;  // @nonnull, @owning

  vec2 size;       // content box
  uint32_t edges;  // a bitfield of enum wlr_edges

  // interaction with the "edge area"
  struct {
    struct wl_signal pressed;  // (struct zn_ui_decoration_edge_pressed_event *)
    struct wl_signal hover;    // (struct zn_ui_decoration_edge_hover_event *)
  } events;
};

/// @param size is the size of content box
void zn_ui_decoration_edge_set_size(
    struct zn_ui_decoration_edge *self, vec2 size);

struct zn_ui_decoration_edge *zn_ui_decoration_edge_create(void);

void zn_ui_decoration_edge_destroy(struct zn_ui_decoration_edge *self);
