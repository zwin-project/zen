#include "mock/backend.h"
#include "mock/output.h"
#include "test-harness.h"
#include "zen-desktop/screen-container.h"
#include "zen-desktop/screen-layout.h"
#include "zen-desktop/shell.h"
#include "zen/server.h"

static void
setup(struct wl_display *display)
{
  struct zn_mock_backend *backend = zn_mock_backend_create();
  zn_server_create(display, &backend->base);
  zn_desktop_shell_create();
}

static void
teardown(void)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_desktop_shell *shell = zn_desktop_shell_get_singleton();
  zn_desktop_shell_destroy(shell);
  zn_server_destroy(server);
}

TEST(add_remove_output)
{
  struct wl_display *display = wl_display_create();
  setup(display);

  struct zn_desktop_shell *shell = zn_desktop_shell_get_singleton();

  struct zn_mock_output *output1 = zn_mock_output_create(1920, 1080);
  struct zn_mock_output *output2 = zn_mock_output_create(720, 480);
  struct zn_mock_output *output3 = zn_mock_output_create(1280, 720);
  struct zn_mock_output *output4 = zn_mock_output_create(1280, 720);

  struct zn_screen_container *container1 =
      zn_screen_container_create(output1->screen);
  struct zn_screen_container *container2 =
      zn_screen_container_create(output2->screen);
  struct zn_screen_container *container3 =
      zn_screen_container_create(output3->screen);
  struct zn_screen_container *container4 =
      zn_screen_container_create(output4->screen);

  zn_screen_layout_add(shell->screen_layout, container1);
  zn_screen_layout_add(shell->screen_layout, container2);
  zn_screen_layout_add(shell->screen_layout, container3);
  zn_screen_layout_add(shell->screen_layout, container4);

  ASSERT_EQUAL_DOUBLE(0, container1->position[0]);
  ASSERT_EQUAL_DOUBLE(0, container1->position[1]);
  ASSERT_EQUAL_DOUBLE(1920, container2->position[0]);
  ASSERT_EQUAL_DOUBLE(0, container2->position[1]);
  ASSERT_EQUAL_DOUBLE(1920 + 720, container3->position[0]);
  ASSERT_EQUAL_DOUBLE(0, container3->position[1]);
  ASSERT_EQUAL_DOUBLE(1920 + 720 + 1280, container4->position[0]);
  ASSERT_EQUAL_DOUBLE(0, container4->position[1]);

  zn_mock_output_destroy(output2);

  ASSERT_EQUAL_DOUBLE(1920, container3->position[0]);
  ASSERT_EQUAL_DOUBLE(0, container3->position[1]);
  ASSERT_EQUAL_DOUBLE(1920 + 1280, container4->position[0]);
  ASSERT_EQUAL_DOUBLE(0, container4->position[1]);

  teardown();
  wl_display_destroy(display);
}

TEST(resize)
{
  struct wl_display *display = wl_display_create();
  setup(display);

  struct zn_desktop_shell *shell = zn_desktop_shell_get_singleton();

  struct zn_mock_output *output1 = zn_mock_output_create(1920, 1080);
  struct zn_mock_output *output2 = zn_mock_output_create(720, 480);
  struct zn_mock_output *output3 = zn_mock_output_create(1280, 720);
  struct zn_mock_output *output4 = zn_mock_output_create(1280, 720);

  struct zn_screen_container *container1 =
      zn_screen_container_create(output1->screen);
  struct zn_screen_container *container2 =
      zn_screen_container_create(output2->screen);
  struct zn_screen_container *container3 =
      zn_screen_container_create(output3->screen);
  struct zn_screen_container *container4 =
      zn_screen_container_create(output4->screen);

  zn_screen_layout_add(shell->screen_layout, container1);
  zn_screen_layout_add(shell->screen_layout, container2);
  zn_screen_layout_add(shell->screen_layout, container3);
  zn_screen_layout_add(shell->screen_layout, container4);

  ASSERT_EQUAL_DOUBLE(0, container1->position[0]);
  ASSERT_EQUAL_DOUBLE(0, container1->position[1]);
  ASSERT_EQUAL_DOUBLE(1920, container2->position[0]);
  ASSERT_EQUAL_DOUBLE(0, container2->position[1]);
  ASSERT_EQUAL_DOUBLE(1920 + 720, container3->position[0]);
  ASSERT_EQUAL_DOUBLE(0, container3->position[1]);
  ASSERT_EQUAL_DOUBLE(1920 + 720 + 1280, container4->position[0]);
  ASSERT_EQUAL_DOUBLE(0, container4->position[1]);

  zn_mock_output_resize(output2, 1920, 1080);

  ASSERT_EQUAL_DOUBLE(1920, container2->position[0]);
  ASSERT_EQUAL_DOUBLE(0, container2->position[1]);
  ASSERT_EQUAL_DOUBLE(1920 + 1920, container3->position[0]);
  ASSERT_EQUAL_DOUBLE(0, container3->position[1]);
  ASSERT_EQUAL_DOUBLE(1920 + 1920 + 1280, container4->position[0]);
  ASSERT_EQUAL_DOUBLE(0, container4->position[1]);

  teardown();
  wl_display_destroy(display);
}
