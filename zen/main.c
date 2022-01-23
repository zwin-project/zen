#include <libzen-compositor/libzen-compositor.h>
#include <signal.h>

static int
on_term_signal(int signal_number, void *data)
{
  struct wl_display *display = data;
  zen_log("\ncaught signal %d\n", signal_number);
  wl_display_terminate(display);
  return 1;
}

int
main()
{
  const char *socket = "zigen-0";
  struct wl_display *display;
  struct wl_event_loop *loop;
  struct wl_event_source *signals[3];
  struct zen_compositor *compositor;
  struct zen_data_device_manager *data_device_manager;
  struct zen_seat *seat;
  int ret, exit = EXIT_FAILURE;

  display = wl_display_create();
  if (display == NULL) {
    zen_log("main: failed to create a display\n");
    goto out;
  }

  compositor = zen_compositor_create(display);
  if (compositor == NULL) {
    zen_log("main: failed to create a compositor\n");
    goto out_compositor;
  }

  seat = zen_seat_create(compositor);
  if (seat == NULL) {
    zen_log("main: failed to create a seat\n");
    goto out_seat;
  }
  compositor->seat = seat;

  data_device_manager = zen_data_device_manager_create(display);
  if (data_device_manager == NULL) {
    zen_log("main: failed to create a data_device_manager\n");
    goto out_data_device_manager;
  }

  ret = zen_compositor_load_shell(compositor);
  if (ret != 0) {
    zen_log("main: failed to load shell\n");
    goto out_shell;
  }

  ret = zen_compositor_load_renderer(compositor);
  if (ret != 0) {
    zen_log("main: failed to load renderer\n");
    goto out_renderer;
  }

  ret = zen_compositor_load_backend(compositor);
  if (ret != 0) {
    zen_log("main: failed to load backend\n");
    goto out_backend;
  }

  ret = wl_display_add_socket(display, socket);
  if (ret != 0) {
    zen_log("main: failed to add socket\n");
    goto out_socket;
  }

  loop = wl_display_get_event_loop(display);
  signals[0] = wl_event_loop_add_signal(loop, SIGTERM, on_term_signal, display);
  signals[1] = wl_event_loop_add_signal(loop, SIGINT, on_term_signal, display);
  signals[2] = wl_event_loop_add_signal(loop, SIGQUIT, on_term_signal, display);

  if (!signals[0] || !signals[1] || !signals[2]) goto out_signal;

  wl_display_run(display);

  exit = EXIT_SUCCESS;

out_signal:
  for (int i = ARRAY_LENGTH(signals) - 1; i >= 0; i--)
    if (signals[i]) wl_event_source_remove(signals[i]);

out_socket:
out_backend:
out_renderer:
out_shell:
  zen_data_device_manager_destroy(data_device_manager);

out_data_device_manager:
  zen_seat_destroy(seat);

out_seat:
  zen_compositor_destroy(compositor);

out_compositor:
  wl_display_destroy(display);

out:
  if (exit == EXIT_SUCCESS)
    zen_log("zen compositor terminated successfully.\n");
  return exit;
}
