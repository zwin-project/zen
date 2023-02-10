#include <signal.h>
#include <stdlib.h>
#include <wayland-server-core.h>

#include "zen-common/log.h"
#include "zen-common/terminate.h"
#include "zen-common/util.h"
#include "zen/include/zen/server.h"

static void
znd_terminate_func(int exit_code, void *data UNUSED)
{
  struct zn_server *server = zn_server_get_singleton();
  zn_server_terminate(server, exit_code);
}

static int
znd_handle_term_signal(int signal_number, void *data UNUSED)
{
  zn_info("Caught signal %d", signal_number);
  zn_terminate(EXIT_FAILURE);

  return 1;
}

int
main(int argc UNUSED, const char *argv[] UNUSED)
{
  int exit_status = EXIT_FAILURE;

  zn_set_terminate_func(znd_terminate_func, NULL);
  zn_log_init(ZEN_DEBUG, zn_terminate);
  struct wl_event_source *signal_sources[4];

  zn_info("Starting %s v%s ...", ZEN_NAME, ZEN_VERSION);

  struct wl_display *display = wl_display_create();
  if (display == NULL) {
    zn_error("Failed to create a wl_display");
    goto err;
  }

  struct wl_event_loop *loop = wl_display_get_event_loop(display);
  signal_sources[0] =
      wl_event_loop_add_signal(loop, SIGTERM, znd_handle_term_signal, display);
  signal_sources[1] =
      wl_event_loop_add_signal(loop, SIGINT, znd_handle_term_signal, display);
  signal_sources[2] =
      wl_event_loop_add_signal(loop, SIGQUIT, znd_handle_term_signal, display);
  signal_sources[3] =
      wl_event_loop_add_signal(loop, SIGCHLD, znd_handle_term_signal, display);

  if (!signal_sources[0] || !signal_sources[1] || !signal_sources[2]) {
    zn_error("Failed to add signal handler");
    goto err_signal;
  }

  struct zn_server *server = zn_server_create(display);
  if (server == NULL) {
    zn_error("Failed to create a zn_server");
    goto err_display;
  }

  exit_status = zn_server_run(server);

err_signal:
  for (int i = ARRAY_LENGTH(signal_sources) - 1; i >= 0; i--) {
    if (signal_sources[i]) {
      wl_event_source_remove(signal_sources[i]);
    }
  }

err_display:
  wl_display_destroy(display);

err:
  zn_info("Exit %s", ZEN_NAME);
  return exit_status;
}
