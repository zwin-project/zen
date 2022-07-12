#include <signal.h>
#include <stdlib.h>
#include <wayland-server.h>

#include "zen-common.h"

struct zn_server {
  struct wl_display *display;
};

static struct zn_server server = {0};
static int exit_status = EXIT_SUCCESS;

static void
zn_terminate(int exit_code)
{
  exit_status = exit_code;
  wl_display_terminate(server.display);
}

static int
on_term_signal(int signal_number, void *data)
{
  struct wl_display *display = data;

  exit_status = EXIT_FAILURE;
  zn_info("Caught signal %d", signal_number);
  wl_display_terminate(display);

  return 1;
}

int
main()
{
  struct wl_event_loop *loop;
  struct wl_event_source *signal_sources[3];
  int i;

  zn_log_init(ZEN_INFO, zn_terminate);

  server.display = wl_display_create();
  if (server.display == NULL) {
    zn_error("Failed to create a wayland display");
    goto err;
  }

  loop = wl_display_get_event_loop(server.display);
  signal_sources[0] =
      wl_event_loop_add_signal(loop, SIGTERM, on_term_signal, server.display);
  signal_sources[1] =
      wl_event_loop_add_signal(loop, SIGINT, on_term_signal, server.display);
  signal_sources[2] =
      wl_event_loop_add_signal(loop, SIGQUIT, on_term_signal, server.display);

  if (!signal_sources[0] || !signal_sources[1] || !signal_sources[2]) {
    zn_error("Failed to add signal handler");
    goto err_signal;
  }

  wl_display_run(server.display);

err_signal:
  for (i = ARRAY_LENGTH(signal_sources) - 1; i >= 0; i--)
    if (signal_sources[i]) wl_event_source_remove(signal_sources[i]);

  wl_display_destroy_clients(server.display);
  wl_display_destroy(server.display);

err:
  zn_info("Exited gracefully");
  return exit_status;
}
