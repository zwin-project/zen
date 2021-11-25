#include <libzen/libzen.h>
#include <sys/time.h>
#include <wayland-server.h>

struct headless_output {
  struct zen_output base;
  struct zen_compositor* compositor;

  uint32_t refresh;
  struct wl_event_source* swap_timer;
};

static void
headless_output_repaint(struct zen_output* output)
{
  UNUSED(output);
}

static int
swap_timer_loop(void* data)
{
  struct headless_output* output = data;
  struct timespec now, next_repaint;
  int64_t refresh_nsec = millihz_to_nsec(output->refresh);
  int64_t next_msec;

  timespec_add_nsec(
      &output->base.frame_time, &output->base.frame_time, refresh_nsec);
  timespec_add_nsec(&next_repaint, &output->base.frame_time, refresh_nsec);

  timespec_get(&now, TIME_UTC);
  next_msec = timespec_sub_to_msec(&next_repaint, &now);
  wl_event_source_timer_update(output->swap_timer, next_msec);

  zen_compositor_finish_frame(output->compositor, next_repaint);

  return 0;
}

WL_EXPORT struct zen_output*
zen_output_create(struct zen_compositor* compositor)
{
  struct headless_output* output;
  struct wl_event_loop* loop;
  struct wl_event_source* swap_timer;
  int32_t refresh = 30000;  // milli hz
  int64_t refresh_nsec = millihz_to_nsec(refresh);

  output = zalloc(sizeof *output);
  if (output == NULL) {
    zen_log("headless output: failed to allocate memory\n");
    goto err;
  }

  loop = wl_display_get_event_loop(compositor->display);
  swap_timer = wl_event_loop_add_timer(loop, swap_timer_loop, output);

  timespec_get(&output->base.frame_time, TIME_UTC);
  timespec_add_nsec(
      &output->base.frame_time, &output->base.frame_time, -refresh_nsec);
  output->base.repaint = headless_output_repaint;
  output->compositor = compositor;
  output->refresh = refresh;
  output->swap_timer = swap_timer;

  // emulate 30 fps display
  swap_timer_loop(output);

  return &output->base;

err:
  return NULL;
}

WL_EXPORT void
zen_output_destroy(struct zen_output* output)
{
  struct headless_output* o = (struct headless_output*)output;

  wl_event_source_remove(o->swap_timer);
  free(output);
}
