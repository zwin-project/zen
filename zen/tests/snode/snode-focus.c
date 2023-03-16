#include <cglm/vec2.h>

#include "test-harness.h"
#include "zen/snode.h"

struct node_data {
  int number;
  bool focused;
};

void
test_on_focus(void *user_data UNUSED, bool focused UNUSED)
{
  struct node_data *data = user_data;

  ASSERT_EQUAL_INT(10, data->number);

  data->focused = focused;
}

const struct zn_snode_interface snode_implementation = {
    .get_texture = zn_snode_noop_get_texture,
    .frame = zn_snode_noop_frame,
    .accepts_input = zn_snode_noop_accepts_input,
    .pointer_button = zn_snode_noop_pointer_button,
    .pointer_enter = zn_snode_noop_pointer_enter,
    .pointer_motion = zn_snode_noop_pointer_motion,
    .pointer_leave = zn_snode_noop_pointer_leave,
    .pointer_axis = zn_snode_noop_pointer_axis,
    .pointer_frame = zn_snode_noop_pointer_frame,
    .on_focus = test_on_focus,
};

TEST(focus)
{
  struct node_data data0 = {0, false};
  struct node_data data1 = {1, false};
  struct node_data data2 = {2, false};
  struct node_data data3 = {3, false};
  struct node_data data_focus = {10, false};

  struct zn_snode *node0 = zn_snode_create(&data0, &snode_implementation);
  struct zn_snode *node1 = zn_snode_create(&data1, &snode_implementation);
  struct zn_snode *node2 = zn_snode_create(&data2, &snode_implementation);
  struct zn_snode *node3 = zn_snode_create(&data3, &snode_implementation);
  struct zn_snode *focusable =
      zn_snode_create_focusable(&data_focus, &snode_implementation);

  // No focusable snode

  zn_snode_set_position(node1, node0, GLM_VEC2_ZERO);
  zn_snode_set_position(node2, node1, GLM_VEC2_ZERO);
  zn_snode_set_position(node3, node2, GLM_VEC2_ZERO);

  ASSERT_EQUAL_POINTER(NULL, zn_snode_get_focusable_parent(node3));

  zn_snode_focus(node3);

  ASSERT_EQUAL_BOOL(false, data0.focused);
  ASSERT_EQUAL_BOOL(false, data1.focused);
  ASSERT_EQUAL_BOOL(false, data2.focused);
  ASSERT_EQUAL_BOOL(false, data3.focused);
  ASSERT_EQUAL_BOOL(false, data_focus.focused);

  // With focusable snode

  zn_snode_set_position(focusable, node0, GLM_VEC2_ZERO);
  zn_snode_set_position(node1, focusable, GLM_VEC2_ZERO);

  ASSERT_EQUAL_POINTER(focusable, zn_snode_get_focusable_parent(node3));

  zn_snode_focus(node3);

  ASSERT_EQUAL_BOOL(false, data0.focused);
  ASSERT_EQUAL_BOOL(false, data1.focused);
  ASSERT_EQUAL_BOOL(false, data2.focused);
  ASSERT_EQUAL_BOOL(false, data3.focused);
  ASSERT_EQUAL_BOOL(true, data_focus.focused);

  zn_snode_unfocus(node3);

  ASSERT_EQUAL_BOOL(false, data0.focused);
  ASSERT_EQUAL_BOOL(false, data1.focused);
  ASSERT_EQUAL_BOOL(false, data2.focused);
  ASSERT_EQUAL_BOOL(false, data3.focused);
  ASSERT_EQUAL_BOOL(false, data_focus.focused);
}
