#include <wlr/render/wlr_texture.h>

#include "mock/output.h"
#include "test-harness.h"
#include "zen-common/util.h"
#include "zen/snode.h"

static struct wlr_texture *
test_get_texture(void *user_data)
{
  return user_data;
}

const struct zn_snode_interface test_impl = {
    .get_texture = test_get_texture,
    .frame = zn_snode_noop_frame,
    .accepts_input = zn_snode_noop_accepts_input,
    .pointer_button = zn_snode_noop_pointer_button,
    .pointer_enter = zn_snode_noop_pointer_enter,
    .pointer_motion = zn_snode_noop_pointer_motion,
    .pointer_leave = zn_snode_noop_pointer_leave,
    .pointer_axis = zn_snode_noop_pointer_axis,
    .pointer_frame = zn_snode_noop_pointer_frame,
};

TEST(set_position)
{
  struct zn_mock_output *output = zn_mock_output_create(0, 0);

  struct wlr_texture texture1;
  texture1.width = 10;
  texture1.height = 10;

  struct wlr_texture texture2;
  texture2.width = 20;
  texture2.height = 20;

  struct zn_snode *root = zn_snode_create_root(output->screen);
  struct zn_snode *node1 = zn_snode_create(&texture1, &test_impl);
  struct zn_snode *node2 = zn_snode_create(&texture2, &test_impl);

  ASSERT_EQUAL_BOOL(
      false, zn_mock_output_damage_contains(output, 50 + 5, 100 + 5));

  zn_snode_set_position(node1, root, (vec2){50, 100});

  ASSERT_EQUAL_BOOL(
      true, zn_mock_output_damage_contains(output, 50 + 5, 100 + 5));

  zn_snode_set_position(node2, node1, (vec2){80, 10});

  ASSERT_EQUAL_BOOL(true,
      zn_mock_output_damage_contains(output, 50 + 80 + 15, 100 + 10 + 15));

  zn_mock_output_damage_clear(output);

  ASSERT_EQUAL_BOOL(false,
      zn_mock_output_damage_contains(output, 50 + 80 + 15, 100 + 10 + 15));
  ASSERT_EQUAL_BOOL(false,
      zn_mock_output_damage_contains(output, 10 + 80 + 15, 20 + 10 + 15));

  zn_snode_set_position(node1, root, (vec2){10, 20});

  ASSERT_EQUAL_BOOL(true,
      zn_mock_output_damage_contains(output, 50 + 80 + 15, 100 + 10 + 15));
  ASSERT_EQUAL_BOOL(
      true, zn_mock_output_damage_contains(output, 10 + 80 + 15, 20 + 10 + 15));
}

TEST(set_position_rebase_parent)
{
  struct zn_mock_output *output = zn_mock_output_create(0, 0);
  struct zn_mock_output *output2 = zn_mock_output_create(0, 0);

  struct wlr_texture texture;
  texture.width = 10;
  texture.height = 10;

  struct zn_snode *root = zn_snode_create_root(output->screen);
  struct zn_snode *root2 = zn_snode_create_root(output2->screen);

  struct zn_snode *node1 = zn_snode_create(&texture, &test_impl);
  struct zn_snode *node2 = zn_snode_create(&texture, &test_impl);
  struct zn_snode *node3 = zn_snode_create(&texture, &test_impl);
  struct zn_snode *node4 = zn_snode_create(&texture, &test_impl);

  zn_snode_set_position(node4, node3, (vec2){100, 100});
  zn_snode_set_position(node3, node2, (vec2){100, 100});
  zn_snode_set_position(node2, node1, (vec2){100, 100});
  zn_snode_set_position(node1, root, (vec2){100, 100});

  zn_mock_output_damage_clear(output);

  ASSERT_EQUAL_BOOL(false, zn_mock_output_damage_contains(output, 405, 405));
  ASSERT_EQUAL_BOOL(false, zn_mock_output_damage_contains(output, 355, 355));

  zn_snode_set_position(node2, root, (vec2){150, 150});

  // damages of node4
  ASSERT_EQUAL_BOOL(true, zn_mock_output_damage_contains(output, 405, 405));
  ASSERT_EQUAL_BOOL(true, zn_mock_output_damage_contains(output, 355, 355));

  /**
   * When the screen associated with a node changes, both screens are damaged.
   */
  zn_mock_output_damage_clear(output);
  zn_mock_output_damage_clear(output2);

  zn_snode_set_position(node2, root2, (vec2){500, 500});

  ASSERT_EQUAL_BOOL(true, zn_mock_output_damage_contains(output, 355, 355));
  ASSERT_EQUAL_BOOL(false, zn_mock_output_damage_contains(output, 705, 705));
  ASSERT_EQUAL_BOOL(true, zn_mock_output_damage_contains(output2, 705, 705));

  /**
   * When a screen is no longer associated with a node, the previous screen is
   * damaged.
   */
  zn_mock_output_damage_clear(output2);

  zn_snode_set_position(node2, NULL, (vec2){500, 500});

  // damages of node4
  ASSERT_EQUAL_BOOL(true, zn_mock_output_damage_contains(output2, 705, 705));

  /**
   * node2 ~ 4 has no screen associated
   */
  zn_mock_output_damage_clear(output2);
  zn_mock_output_damage_clear(output);

  zn_snode_set_position(node4, node3, (vec2){300, 300});

  // damages of node4
  ASSERT_EQUAL_BOOL(false, pixman_region32_not_empty(&output->damage));
  ASSERT_EQUAL_BOOL(false, pixman_region32_not_empty(&output2->damage));
}

TEST(damage)
{
  struct zn_mock_output *output = zn_mock_output_create(0, 0);

  struct wlr_texture texture;
  texture.width = 50;
  texture.height = 50;

  struct zn_snode *root = zn_snode_create_root(output->screen);

  struct zn_snode *node1 = zn_snode_create(&texture, &test_impl);
  struct zn_snode *node2 = zn_snode_create(&texture, &test_impl);
  struct zn_snode *node3 = zn_snode_create(&texture, &test_impl);
  struct zn_snode *node4 = zn_snode_create(&texture, &test_impl);

  zn_snode_set_position(node3, node2, (vec2){200, 100});
  zn_snode_set_position(node2, node1, (vec2){100, 200});
  zn_snode_set_position(node1, root, (vec2){100, 100});

  zn_mock_output_damage_clear(output);

  ASSERT_EQUAL_BOOL(false, zn_mock_output_damage_contains(output, 115, 135));
  zn_snode_damage(node1, &(struct wlr_fbox){10, 30, 10, 10});
  ASSERT_EQUAL_BOOL(true, zn_mock_output_damage_contains(output, 115, 135));

  ASSERT_EQUAL_BOOL(false, zn_mock_output_damage_contains(output, 435, 425));
  zn_snode_damage(node3, &(struct wlr_fbox){30, 20, 10, 10});
  ASSERT_EQUAL_BOOL(true, zn_mock_output_damage_contains(output, 435, 425));

  zn_mock_output_damage_clear(output);

  zn_snode_damage(node4, &(struct wlr_fbox){10, 30, 10, 10});
  ASSERT_EQUAL_BOOL(false, pixman_region32_not_empty(&output->damage));
}
