#include "zen-desktop/animation.h"

#include <bits/time.h>
#include <time.h>

#include "zen-common/log.h"
#include "zen-common/timespec-util.h"
#include "zen-common/util.h"

void
zn_animation_start(struct zn_animation *self, float value, int32_t duration_ms)
{
  self->animating = true;
  self->start_value = self->value;
  self->target_value = value;
  self->duration_ms = duration_ms;
  clock_gettime(CLOCK_MONOTONIC, &self->start_time);

  wl_signal_emit(&self->events.frame, NULL);
}

void
zn_animation_notify_frame(struct zn_animation *self)
{
  if (!self->animating) {
    return;
  }

  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);

  int64_t passed_ms = timespec_sub_to_msec(&now, &self->start_time);

  if (self->duration_ms < passed_ms) {
    self->animating = false;
    self->value = self->target_value;
    wl_signal_emit(&self->events.frame, NULL);
    wl_signal_emit(&self->events.done, NULL);
    return;
  }

  float interpolation = (float)passed_ms / (float)self->duration_ms;

  self->value = (1 - interpolation) * self->start_value +
                interpolation * self->target_value;

  wl_signal_emit(&self->events.frame, NULL);
}

struct zn_animation *
zn_animation_create(float initial_value)
{
  struct zn_animation *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->value = initial_value;

  self->duration_ms = 0;
  clock_gettime(CLOCK_MONOTONIC, &self->start_time);

  self->animating = false;

  wl_signal_init(&self->events.cancel);
  wl_signal_init(&self->events.done);
  wl_signal_init(&self->events.frame);

  return self;

err:
  return NULL;
}

void
zn_animation_destroy(struct zn_animation *self)
{
  wl_list_remove(&self->events.cancel.listener_list);
  wl_list_remove(&self->events.done.listener_list);
  wl_list_remove(&self->events.frame.listener_list);
  free(self);
}
