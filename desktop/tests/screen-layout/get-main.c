#include "setup.h"
#include "test-harness.h"
#include "zen-desktop/screen-layout.h"
#include "zen-desktop/screen.h"

TEST(get_main)
{
  struct wl_display *display = wl_display_create();
  setup(display);

  struct zn_desktop_shell *shell = zn_desktop_shell_get_singleton();

  struct zn_mock_output *output1 = zn_mock_output_create(1920, 1080);
  struct zn_mock_output *output2 = zn_mock_output_create(720, 480);
  struct zn_mock_output *output3 = zn_mock_output_create(1280, 720);

  struct zn_desktop_screen *desktop_screen1 =
      zn_desktop_screen_create(output1->screen);
  struct zn_desktop_screen *desktop_screen2 =
      zn_desktop_screen_create(output2->screen);
  struct zn_desktop_screen *desktop_screen3 =
      zn_desktop_screen_create(output3->screen);

  ASSERT_EQUAL_POINTER(
      NULL, zn_screen_layout_get_main_screen(shell->screen_layout));

  zn_screen_layout_add(shell->screen_layout, desktop_screen1);
  ASSERT_EQUAL_POINTER(
      desktop_screen1, zn_screen_layout_get_main_screen(shell->screen_layout));

  zn_screen_layout_add(shell->screen_layout, desktop_screen2);
  ASSERT_EQUAL_POINTER(
      desktop_screen1, zn_screen_layout_get_main_screen(shell->screen_layout));

  zn_screen_layout_add(shell->screen_layout, desktop_screen3);
  ASSERT_EQUAL_POINTER(
      desktop_screen1, zn_screen_layout_get_main_screen(shell->screen_layout));

  zn_mock_output_destroy(output1);
  ASSERT_EQUAL_POINTER(
      desktop_screen2, zn_screen_layout_get_main_screen(shell->screen_layout));

  zn_mock_output_destroy(output2);
  ASSERT_EQUAL_POINTER(
      desktop_screen3, zn_screen_layout_get_main_screen(shell->screen_layout));

  zn_mock_output_destroy(output3);
  ASSERT_EQUAL_POINTER(
      NULL, zn_screen_layout_get_main_screen(shell->screen_layout));

  teardown();
  wl_display_destroy(display);
}
