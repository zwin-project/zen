#include <signal.h>
#include <stdlib.h>
#include <wayland-server.h>
#include <zen-util.h>

static int
on_term_signal(int signal_number, void *data)
{
  struct wl_display *display = data;

  zn_log("caught signal %d\n", signal_number);
  wl_display_terminate(display);

  return 1;
}

int
main()
{
  struct wl_display *display;
  struct wl_event_loop *loop;
  struct wl_event_source *signal_sources[3];
  int i, status = EXIT_FAILURE;

  display = wl_display_create();
  if (display == NULL) {
    zn_log("main: failed to load\n");
    goto err;
  }

  loop = wl_display_get_event_loop(display);
  signal_sources[0] =
      wl_event_loop_add_signal(loop, SIGTERM, on_term_signal, display);
  signal_sources[1] =
      wl_event_loop_add_signal(loop, SIGINT, on_term_signal, display);
  signal_sources[2] =
      wl_event_loop_add_signal(loop, SIGQUIT, on_term_signal, display);

  if (!signal_sources[0] || !signal_sources[1] || !signal_sources[2]) {
    zn_log("main: failed to add signal handler\n");
    goto err_signal;
  }

  wl_display_run(display);

  status = EXIT_SUCCESS;

err_signal:
  for (i = ARRAY_LENGTH(signal_sources) - 1; i >= 0; i--)
    if (signal_sources[i]) wl_event_source_remove(signal_sources[i]);

  wl_display_destroy(display);

err:
  zn_log("Exited gracefully\n");
  return status;
}
