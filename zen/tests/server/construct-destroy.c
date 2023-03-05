#include <wayland-server-core.h>

#include "mock/backend.h"
#include "test-harness.h"
#include "zen/cursor.h"
#include "zen/seat.h"
#include "zen/server.h"

TEST(construct_destroy)
{
  struct zn_mock_backend *backend = zn_mock_backend_create();
  struct wl_display *display = wl_display_create();
  struct zn_server *server = zn_server_create(display, &backend->base);

  ASSERT_NOT_EQUAL_POINTER(NULL, server);

  zn_server_destroy(server);
}
