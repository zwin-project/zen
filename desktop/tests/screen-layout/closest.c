#include <cglm/vec2.h>

#include "setup.h"
#include "test-harness.h"
#include "zen-desktop/screen-layout.h"
#include "zen-desktop/screen.h"

TEST(get_closest_position)
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

  zn_screen_layout_add(shell->screen_layout, desktop_screen1);
  zn_screen_layout_add(shell->screen_layout, desktop_screen2);
  zn_screen_layout_add(shell->screen_layout, desktop_screen3);

  struct zn_desktop_screen *desktop_screen_out = NULL;
  vec2 position;

  /**
   * Inside the screen.
   */
  glm_vec2_copy((vec2){100, 200}, position);
  zn_screen_layout_get_closest_position(shell->screen_layout, desktop_screen1,
      position, &desktop_screen_out, position);

  ASSERT_EQUAL_POINTER(desktop_screen1, desktop_screen_out);
  ASSERT_EQUAL_DOUBLE(100, position[0]);
  ASSERT_EQUAL_DOUBLE(200, position[1]);

  glm_vec2_copy((vec2){700, 480}, position);
  zn_screen_layout_get_closest_position(shell->screen_layout, desktop_screen2,
      position, &desktop_screen_out, position);
  ASSERT_EQUAL_POINTER(desktop_screen2, desktop_screen_out);
  ASSERT_EQUAL_DOUBLE(700, position[0]);
  ASSERT_EQUAL_DOUBLE(480, position[1]);

  /**
   * Outside the screen, and the closest screen is the same screen.
   */
  glm_vec2_copy((vec2){-100, -200}, position);
  zn_screen_layout_get_closest_position(shell->screen_layout, desktop_screen1,
      position, &desktop_screen_out, position);
  ASSERT_EQUAL_POINTER(desktop_screen1, desktop_screen_out);
  ASSERT_EQUAL_DOUBLE(0, position[0]);
  ASSERT_EQUAL_DOUBLE(0, position[1]);

  glm_vec2_copy((vec2){100, 1100}, position);
  zn_screen_layout_get_closest_position(shell->screen_layout, desktop_screen1,
      position, &desktop_screen_out, position);
  ASSERT_EQUAL_POINTER(desktop_screen1, desktop_screen_out);
  ASSERT_EQUAL_DOUBLE(100, position[0]);
  ASSERT_EQUAL_DOUBLE(1080, position[1]);

  glm_vec2_copy((vec2){300, 500}, position);
  zn_screen_layout_get_closest_position(shell->screen_layout, desktop_screen2,
      position, &desktop_screen_out, position);
  ASSERT_EQUAL_POINTER(desktop_screen2, desktop_screen_out);
  ASSERT_EQUAL_DOUBLE(300, position[0]);
  ASSERT_EQUAL_DOUBLE(480, position[1]);

  /**
   * Outside the screen, and the closest screen is the different screen.
   */
  glm_vec2_copy((vec2){1920 + 200, 480 + 100}, position);
  zn_screen_layout_get_closest_position(shell->screen_layout, desktop_screen1,
      position, &desktop_screen_out, position);
  ASSERT_EQUAL_POINTER(desktop_screen2, desktop_screen_out);
  ASSERT_EQUAL_DOUBLE(200, position[0]);
  ASSERT_EQUAL_DOUBLE(480, position[1]);

  /**
   * Inside the different screen
   */
  glm_vec2_copy((vec2){1920 + 200, 380}, position);
  zn_screen_layout_get_closest_position(shell->screen_layout, desktop_screen1,
      position, &desktop_screen_out, position);
  ASSERT_EQUAL_POINTER(desktop_screen2, desktop_screen_out);
  ASSERT_EQUAL_DOUBLE(200, position[0]);
  ASSERT_EQUAL_DOUBLE(380, position[1]);

  /**
   * Border
   */
  glm_vec2_copy((vec2){1920, 380}, position);
  zn_screen_layout_get_closest_position(shell->screen_layout, desktop_screen1,
      position, &desktop_screen_out, position);
  ASSERT_EQUAL_BOOL(
      true, (desktop_screen_out == desktop_screen1 && position[0] == 1920) ||
                (desktop_screen_out == desktop_screen2 && position[1] == 0));
  ASSERT_EQUAL_DOUBLE(380, position[1]);

  teardown();
  wl_display_destroy(display);
}
