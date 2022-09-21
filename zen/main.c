#include <getopt.h>
#include <linux/input.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include <wlr/util/log.h>

#include "zen-common.h"
#include "zen/server.h"

static pid_t startup_command_pid = -1;

static zn_log_importance_t
convert_wlr_log_importance(enum wlr_log_importance importance)
{
  switch (importance) {
    case WLR_ERROR:
      return ZEN_ERROR;
    case WLR_INFO:
      return ZEN_INFO;
    default:
      return ZEN_DEBUG;
  }
}

static void
handle_wlr_log(
    enum wlr_log_importance importance, const char *fmt, va_list args)
{
  static char zn_fmt[1024];
  snprintf(zn_fmt, sizeof(zn_fmt), "[wlr] %s", fmt);
  _zn_vlog(convert_wlr_log_importance(importance), zn_fmt, args);
}

static void
zn_terminate(int exit_code)
{
  struct zn_server *server = zn_server_get_singleton();
  zn_server_terminate(server, exit_code);
}

static void
zn_terminate_binding_handler(uint32_t time_msec, uint32_t key, void *data)
{
  struct zn_server *server = zn_server_get_singleton();
  UNUSED(time_msec);
  UNUSED(key);
  UNUSED(data);

  zn_server_terminate(server, EXIT_SUCCESS);
}

static void
zn_switch_vt_handler(uint32_t time_msec, uint32_t key, void *data)
{
  UNUSED(data);
  UNUSED(time_msec);

  if (!zn_assert(KEY_F1 <= key && key <= KEY_F10,
          "Don't assign this keybind to outside F1-F10")) {
    return;
  }

  const unsigned int vt = key - KEY_F1 + 1;
  struct zn_server *server = zn_server_get_singleton();
  struct wlr_session *session = wlr_backend_get_session(server->backend);

  if (!session) {
    return;
  }

  wlr_session_change_vt(session, vt);
}

static int
on_term_signal(int signal_number, void *data)
{
  struct zn_server *server = zn_server_get_singleton();
  UNUSED(data);

  zn_info("Caught signal %d", signal_number);
  zn_server_terminate(server, EXIT_FAILURE);

  return 1;
}

static int
on_signal_child(int signal_number, void *data)
{
  struct zn_server *server = zn_server_get_singleton();
  pid_t pid;
  int status;
  UNUSED(data);
  UNUSED(signal_number);

  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    if (startup_command_pid != -1 && pid == startup_command_pid) {
      zn_debug("Startup command exited");
      zn_server_terminate(server, EXIT_SUCCESS);
    }
  }

  if (pid < 0 && errno != ECHILD) zn_error("waitpid error %s", strerror(errno));

  return 1;
}

static pid_t
launch_startup_command(char *command)
{
  pid_t pid = -1;

  pid = fork();
  if (pid == -1) {
    zn_error("Failed to fork startup command process: %s", strerror(errno));
    goto err;
  } else if (pid == 0) {
    execl("/bin/sh", "/bin/sh", "-c", command, NULL);
    fprintf(stderr, "Failed to execute startup command (%s): %s\n", command,
        strerror(errno));
    _exit(EXIT_FAILURE);
  }

err:
  return pid;
}

int
main(int argc, char *argv[])
{
  struct wl_display *display;
  struct wl_event_loop *loop;
  struct wl_event_source *signal_sources[4];
  int i, exit_status = EXIT_FAILURE;
  char *startup_command = NULL;
  struct zn_server *server;

  static const struct option long_options[] = {
      {"help", no_argument, NULL, 'h'},
      {"startup-command", required_argument, NULL, 's'},
      {0, 0, 0, 0},
  };

  const char *usage =
      "Usage: zen-desktop [options]"
      "\n"
      "  -h --help             Show help message and quit.\n"
      "  -s --startup-command  Specify startup command.\n"
      "\n";

  while (1) {
    int c, option_index = 0;
    c = getopt_long(argc, argv, "hs:", long_options, &option_index);
    if (c == -1) break;

    switch (c) {
      case 'h':  // help
        printf("%s", usage);
        exit(EXIT_SUCCESS);
        break;
      case 's':  // startup command
        startup_command = strdup(optarg);
        break;
      default:
        fprintf(stderr, "%s", usage);
        exit(EXIT_FAILURE);
        break;
    }
  }

  if (optind < argc) {
    fprintf(stderr, "%s", usage);
    exit(EXIT_FAILURE);
  }

  zn_log_init(ZEN_DEBUG, zn_terminate);
  wlr_log_init(WLR_DEBUG, handle_wlr_log);

  display = wl_display_create();
  if (display == NULL) {
    zn_error("Failed to create a wayland display");
    goto err;
  }

  loop = wl_display_get_event_loop(display);
  signal_sources[0] =
      wl_event_loop_add_signal(loop, SIGTERM, on_term_signal, display);
  signal_sources[1] =
      wl_event_loop_add_signal(loop, SIGINT, on_term_signal, display);
  signal_sources[2] =
      wl_event_loop_add_signal(loop, SIGQUIT, on_term_signal, display);
  signal_sources[3] =
      wl_event_loop_add_signal(loop, SIGCHLD, on_signal_child, display);

  if (!signal_sources[0] || !signal_sources[1] || !signal_sources[2]) {
    zn_error("Failed to add signal handler");
    goto err_signal;
  }

  server = zn_server_create(display);
  if (server == NULL) {
    zn_error("Failed to create a zen server");
    goto err_signal;
  }

  // Terminate the program with a keyboard event for development convenience.
  zn_input_manager_add_key_binding(server->input_manager, KEY_Q,
      WLR_MODIFIER_ALT, zn_terminate_binding_handler, NULL);

  for (int i = KEY_F1; i <= KEY_F10; ++i) {
    zn_input_manager_add_key_binding(server->input_manager, i,
        WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, zn_switch_vt_handler, NULL);
  }

  if (startup_command) {
    startup_command_pid = launch_startup_command(startup_command);
    if (startup_command_pid < 0) goto err_server;
  }

  exit_status = zn_server_run(server);

err_server:
  zn_server_destroy(server);
  server = NULL;

err_signal:
  for (i = ARRAY_LENGTH(signal_sources) - 1; i >= 0; i--)
    if (signal_sources[i]) wl_event_source_remove(signal_sources[i]);

  wl_display_destroy_clients(display);
  wl_display_destroy(display);

err:
  free(startup_command);
  startup_command = NULL;

  zn_info("Exited gracefully");
  return exit_status;
}
