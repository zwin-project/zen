#pragma once

#include "mock/backend.h"
#include "mock/output.h"
#include "zen-common/util.h"
#include "zen-desktop/shell.h"
#include "zen/server.h"

UNUSED static void
setup(struct wl_display *display)
{
  struct zn_mock_backend *backend = zn_mock_backend_create();
  zn_server_create(display, &backend->base);
  zn_desktop_shell_create();
}

UNUSED static void
teardown(void)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_desktop_shell *shell = zn_desktop_shell_get_singleton();
  zn_desktop_shell_destroy(shell);
  zn_server_destroy(server);
}
