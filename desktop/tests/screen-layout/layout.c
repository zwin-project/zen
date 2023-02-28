#include "mock/backend.h"
#include "mock/output.h"
#include "setup.h"
#include "test-harness.h"
#include "zen-desktop/screen-layout.h"
#include "zen-desktop/screen.h"
#include "zen-desktop/shell.h"

TEST(add_remove_output)
{
  struct wl_display *display = wl_display_create();
  setup(display);

  struct zn_desktop_shell *shell = zn_desktop_shell_get_singleton();

  struct zn_mock_output *output1 = zn_mock_output_create(1920, 1080);
  struct zn_mock_output *output2 = zn_mock_output_create(720, 480);
  struct zn_mock_output *output3 = zn_mock_output_create(1280, 720);
  struct zn_mock_output *output4 = zn_mock_output_create(1280, 720);

  struct zn_desktop_screen *desktop_screen1 =
      zn_desktop_screen_create(output1->screen);
  struct zn_desktop_screen *desktop_screen2 =
      zn_desktop_screen_create(output2->screen);
  struct zn_desktop_screen *desktop_screen3 =
      zn_desktop_screen_create(output3->screen);
  struct zn_desktop_screen *desktop_screen4 =
      zn_desktop_screen_create(output4->screen);

  zn_screen_layout_add(shell->screen_layout, desktop_screen1);
  zn_screen_layout_add(shell->screen_layout, desktop_screen2);
  zn_screen_layout_add(shell->screen_layout, desktop_screen3);
  zn_screen_layout_add(shell->screen_layout, desktop_screen4);

  ASSERT_EQUAL_DOUBLE(0, desktop_screen1->position[0]);
  ASSERT_EQUAL_DOUBLE(0, desktop_screen1->position[1]);
  ASSERT_EQUAL_DOUBLE(1920, desktop_screen2->position[0]);
  ASSERT_EQUAL_DOUBLE(0, desktop_screen2->position[1]);
  ASSERT_EQUAL_DOUBLE(1920 + 720, desktop_screen3->position[0]);
  ASSERT_EQUAL_DOUBLE(0, desktop_screen3->position[1]);
  ASSERT_EQUAL_DOUBLE(1920 + 720 + 1280, desktop_screen4->position[0]);
  ASSERT_EQUAL_DOUBLE(0, desktop_screen4->position[1]);

  zn_mock_output_destroy(output2);

  ASSERT_EQUAL_DOUBLE(1920, desktop_screen3->position[0]);
  ASSERT_EQUAL_DOUBLE(0, desktop_screen3->position[1]);
  ASSERT_EQUAL_DOUBLE(1920 + 1280, desktop_screen4->position[0]);
  ASSERT_EQUAL_DOUBLE(0, desktop_screen4->position[1]);

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

  struct zn_desktop_screen *desktop_screen1 =
      zn_desktop_screen_create(output1->screen);
  struct zn_desktop_screen *desktop_screen2 =
      zn_desktop_screen_create(output2->screen);
  struct zn_desktop_screen *desktop_screen3 =
      zn_desktop_screen_create(output3->screen);
  struct zn_desktop_screen *desktop_screen4 =
      zn_desktop_screen_create(output4->screen);

  zn_screen_layout_add(shell->screen_layout, desktop_screen1);
  zn_screen_layout_add(shell->screen_layout, desktop_screen2);
  zn_screen_layout_add(shell->screen_layout, desktop_screen3);
  zn_screen_layout_add(shell->screen_layout, desktop_screen4);

  ASSERT_EQUAL_DOUBLE(0, desktop_screen1->position[0]);
  ASSERT_EQUAL_DOUBLE(0, desktop_screen1->position[1]);
  ASSERT_EQUAL_DOUBLE(1920, desktop_screen2->position[0]);
  ASSERT_EQUAL_DOUBLE(0, desktop_screen2->position[1]);
  ASSERT_EQUAL_DOUBLE(1920 + 720, desktop_screen3->position[0]);
  ASSERT_EQUAL_DOUBLE(0, desktop_screen3->position[1]);
  ASSERT_EQUAL_DOUBLE(1920 + 720 + 1280, desktop_screen4->position[0]);
  ASSERT_EQUAL_DOUBLE(0, desktop_screen4->position[1]);

  zn_mock_output_resize(output2, 1920, 1080);

  ASSERT_EQUAL_DOUBLE(1920, desktop_screen2->position[0]);
  ASSERT_EQUAL_DOUBLE(0, desktop_screen2->position[1]);
  ASSERT_EQUAL_DOUBLE(1920 + 1920, desktop_screen3->position[0]);
  ASSERT_EQUAL_DOUBLE(0, desktop_screen3->position[1]);
  ASSERT_EQUAL_DOUBLE(1920 + 1920 + 1280, desktop_screen4->position[0]);
  ASSERT_EQUAL_DOUBLE(0, desktop_screen4->position[1]);

  teardown();
  wl_display_destroy(display);
}
