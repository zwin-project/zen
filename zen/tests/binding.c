#include "binding.h"

#include <linux/input.h>

#include "mock/backend.h"
#include "test-harness.h"
#include "zen/config.h"
#include "zen/server.h"

static struct zn_server *
setup(void)
{
  struct zn_mock_backend *backend = zn_mock_backend_create();
  struct wl_display *display = wl_display_create();
  struct zn_config *config = zn_config_create();
  struct zn_server *server = zn_server_create(display, &backend->base, config);
  return server;
}

static void
teardown(struct zn_server *server)
{
  struct zn_config *config = server->config;
  zn_server_destroy(server);
  zn_config_destroy(config);
}

static bool
handle_test1(const char *name UNUSED, uint32_t key UNUSED,
    uint32_t modifiers UNUSED, void *user_data)
{
  bool *test1 = (bool *)user_data;
  *test1 = true;

  return true;
}

static bool
handle_test2(const char *name UNUSED, uint32_t key UNUSED,
    uint32_t modifiers UNUSED, void *user_data)
{
  bool *test2 = (bool *)user_data;
  *test2 = true;

  return true;
}

TEST(binding)
{
  struct zn_server *server = setup();

  bool test1 = false;
  bool test2 = false;

  zn_binding_add(server->binding, "test1", handle_test1, &test1);
  zn_binding_add(server->binding, "test2", handle_test2, &test2);

  zn_binding_remap(server->binding);

  zn_binding_handle_key(server->binding, KEY_4, WLR_MODIFIER_ALT);
  ASSERT_EQUAL_BOOL(true, test1);
  ASSERT_EQUAL_BOOL(false, test2);

  zn_binding_handle_key(
      server->binding, KEY_4, WLR_MODIFIER_ALT | WLR_MODIFIER_LOGO);
  ASSERT_EQUAL_BOOL(true, test1);
  ASSERT_EQUAL_BOOL(true, test2);

  teardown(server);
}

TEST(duplicated)
{
  struct zn_server *server = setup();

  bool test1 = false;
  bool test2 = false;

  zn_binding_add(server->binding, "test1", handle_test1, &test1);
  zn_binding_add(server->binding, "test1", handle_test1, &test1);
  zn_binding_add(server->binding, "test2", handle_test2, &test2);

  ulong item_count =
      server->binding->items.size / sizeof(struct zn_binding_item);

  ASSERT_EQUAL_INT(2, item_count);

  teardown(server);
}
