#include <cglm/vec2.h>
#include <stdio.h>
#include <wlr/util/box.h>

#include "mock/output.h"
#include "test-harness.h"
#include "zen-common/wlr/box.h"
#include "zen/snode-root.h"
#include "zen/snode.h"

bool
test_accepts_input(void *user_data UNUSED, const vec2 point UNUSED)
{
  struct wlr_fbox *box = user_data;
  return zn_wlr_fbox_contains_point(box, point[0], point[1]);
}

const struct zn_snode_interface test_impl = {
    .get_texture = zn_snode_noop_get_texture,
    .frame = zn_snode_noop_frame,
    .accepts_input = test_accepts_input,
    .pointer_button = zn_snode_noop_pointer_button,
    .pointer_enter = zn_snode_noop_pointer_enter,
    .pointer_motion = zn_snode_noop_pointer_motion,
    .pointer_leave = zn_snode_noop_pointer_leave,
    .pointer_axis = zn_snode_noop_pointer_axis,
    .pointer_frame = zn_snode_noop_pointer_frame,
    .on_focus = zn_snode_noop_on_focus,
};

#define create_snode_with_input_region(x, y, width, height) \
  zn_snode_create(&(struct wlr_fbox){x, y, width, height}, &test_impl)

///     300                      800       1000     1200
///  500 +------------------------.---------.---------+
///      |node1                   .         .         |
///      |                        .         .         |
///      |                        .         .         |
///      |                        +---------+ . . . . . 700
///      |    (origin of 3)       |3        |         |
///  800 . . .+                   |         |         |
///      |    .                   |         |         |
///  900 . . . . . . . +----------|         |         |
///      |    .        |2         |         |         |
///      |    .        |          +---------+ . . . . . 1000
///      |    .        |               |              |
///      |    .        |               |              |
///      |    .        |               |              |
///      |    .        +---------------+. . . . . . . . 1200
///      |    .        .               .              |
/// 1300 +----.--------.---------------.--------------+
///          400      600             900
TEST(normal)
{
  struct zn_mock_output *output = zn_mock_output_create(0, 0);

  struct zn_snode *root = output->screen->snode_root->node;
  struct zn_snode *node1 = create_snode_with_input_region(0, 0, 900, 800);
  struct zn_snode *node2 = create_snode_with_input_region(0, 0, 300, 300);
  struct zn_snode *node3 = create_snode_with_input_region(400, -100, 200, 300);

  zn_snode_set_position(node1, root, (vec2){300, 500});
  zn_snode_set_position(node2, node1, (vec2){300, 400});
  zn_snode_set_position(node3, node1, (vec2){100, 300});

  vec2 local_point = GLM_VEC2_ONE_INIT;
  struct zn_snode *node = NULL;

  node = zn_snode_get_snode_at(root, (vec2){100, 800}, local_point);
  ASSERT_EQUAL_POINTER(NULL, node);
  ASSERT_EQUAL_VEC2(GLM_VEC2_ONE, local_point);

  node = zn_snode_get_snode_at(root, (vec2){400, 800}, local_point);
  ASSERT_EQUAL_POINTER(node1, node);
  ASSERT_EQUAL_VEC2(((vec2){100, 300}), local_point);

  node = zn_snode_get_snode_at(root, (vec2){700, 1100}, local_point);
  ASSERT_EQUAL_POINTER(node2, node);
  ASSERT_EQUAL_VEC2(((vec2){100, 200}), local_point);

  node = zn_snode_get_snode_at(root, (vec2){950, 900}, local_point);
  ASSERT_EQUAL_POINTER(node3, node);
  ASSERT_EQUAL_VEC2(((vec2){950 - 400, 900 - 800}), local_point);

  node = zn_snode_get_snode_at(root, (vec2){850, 960}, local_point);
  ASSERT_EQUAL_POINTER(node3, node);
  ASSERT_EQUAL_VEC2(((vec2){850 - 400, 960 - 800}), local_point);

  // move node1 (+100, +100)
  zn_snode_set_position(node1, root, (vec2){400, 600});

  glm_vec2_copy(GLM_VEC2_ONE, local_point);

  node = zn_snode_get_snode_at(root, (vec2){100 + 100, 800 + 100}, local_point);
  ASSERT_EQUAL_POINTER(NULL, node);
  ASSERT_EQUAL_VEC2(GLM_VEC2_ONE, local_point);

  node = zn_snode_get_snode_at(root, (vec2){400 + 100, 800 + 100}, local_point);
  ASSERT_EQUAL_POINTER(node1, node);
  ASSERT_EQUAL_VEC2(((vec2){100, 300}), local_point);

  node =
      zn_snode_get_snode_at(root, (vec2){700 + 100, 1100 + 100}, local_point);
  ASSERT_EQUAL_POINTER(node2, node);
  ASSERT_EQUAL_VEC2(((vec2){100, 200}), local_point);

  node = zn_snode_get_snode_at(root, (vec2){950 + 100, 900 + 100}, local_point);
  ASSERT_EQUAL_POINTER(node3, node);
  ASSERT_EQUAL_VEC2(((vec2){950 - 400, 900 - 800}), local_point);

  node = zn_snode_get_snode_at(root, (vec2){850 + 100, 960 + 100}, local_point);
  ASSERT_EQUAL_POINTER(node3, node);
  ASSERT_EQUAL_VEC2(((vec2){850 - 400, 960 - 800}), local_point);
}

///    500            800  900  1000
/// 300 +--------------.----+    .
///     |node1         .    |    .
/// 400 . . . . . . . .+---------+
///     |              |2        |
///     |              |         |
///     |              |         |
/// 600 . . . . . . . .+---------+
///     |                   |
/// 700 +-------------------+
///
TEST(stick_out)
{
  struct zn_snode *root = create_snode_with_input_region(0, 0, 0, 0);
  struct zn_snode *node1 = create_snode_with_input_region(0, 0, 400, 400);
  struct zn_snode *node2 = create_snode_with_input_region(0, 0, 200, 200);

  zn_snode_set_position(node1, root, (vec2){500, 300});
  zn_snode_set_position(node2, node1, (vec2){300, 100});

  vec2 local_point = GLM_VEC2_ONE_INIT;
  struct zn_snode *node = NULL;

  node = zn_snode_get_snode_at(root, (vec2){950, 350}, local_point);
  ASSERT_EQUAL_POINTER(NULL, node);
  ASSERT_EQUAL_VEC2(GLM_VEC2_ONE, local_point);

  node = zn_snode_get_snode_at(root, (vec2){950, 550}, local_point);
  ASSERT_EQUAL_POINTER(node2, node);
  ASSERT_EQUAL_VEC2(((vec2){150, 150}), local_point);
}
