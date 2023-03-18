#pragma once

#include <wayland-server-core.h>

struct zn_animation {
  float value;

  float start_value;
  float target_value;

  struct timespec start_time;
  int64_t duration_ms;

  bool animating;

  struct {
    struct wl_signal frame;   // (NULL)
    struct wl_signal done;    // (NULL)
    struct wl_signal cancel;  // (NULL)
  } events;
};

void zn_animation_start(
    struct zn_animation *self, float value, int32_t duration_ms);

void zn_animation_notify_frame(struct zn_animation *self);

struct zn_animation *zn_animation_create(float initial_value);

void zn_animation_destroy(struct zn_animation *self);
