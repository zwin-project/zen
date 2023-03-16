#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_seat.h>

struct zn_cursor;
struct zn_snode;

struct zn_seat {
  struct wlr_seat *wlr_seat;  // @nonnull, @owning

  struct zn_cursor *cursor;  // @nonnull, @owning

  struct {
    struct zn_snode *focus;  // @nullable, @ref
    int button_count;
  } pointer_state;

  struct zn_snode *focus;  // @nullable @ref

  struct wl_listener cursor_focus_destroy_listener;
  struct wl_listener focus_destroy_listener;
  struct wl_listener set_cursor_listener;

  struct {
    struct wl_signal pointer_motion;  // (struct wlr_event_pointer_motion *)
    struct wl_signal pointer_button;  // (struct wlr_event_pointer_button)
    struct wl_signal pointer_axis;    // (struct wlr_event_pointer_axis)
    struct wl_signal pointer_frame;   // (NULL)

    // (uint32_t *) pointer to a bitfield of enum wl_seat_capability
    struct wl_signal update_capabilities;
  } events;
};

/// @return value can be NULL
struct zn_screen *zn_seat_get_focused_screen(struct zn_seat *self);

/// @param capabilities is a bitfield of enum wl_seat_capability
void zn_seat_set_capabilities(struct zn_seat *self, uint32_t capabilities);

/// @param snode is nullable
void zn_seat_set_focus(struct zn_seat *self, struct zn_snode *snode);

void zn_seat_pointer_button(struct zn_seat *self, uint32_t time_msec,
    uint32_t button, enum wlr_button_state state);

/// @param point is in snode-local coords
/// @param snode is nullable
void zn_seat_pointer_enter(
    struct zn_seat *self, struct zn_snode *snode, vec2 point);

/// @param point is in snode-local coords
void zn_seat_pointer_motion(
    struct zn_seat *self, uint32_t time_msec, vec2 point);

void zn_seat_pointer_axis(struct zn_seat *self, uint32_t time_msec,
    enum wlr_axis_source source, enum wlr_axis_orientation orientation,
    double delta, int32_t delta_discrete);

void zn_seat_pointer_frame(struct zn_seat *self);
