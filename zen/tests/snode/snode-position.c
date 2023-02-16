#include "snode.c"  // NOLINT(bugprone-suspicious-include)
#include "test-harness.h"
#include "zen-common/util.h"

static struct wlr_texture *
get_texture(void *user_data UNUSED)
{
  return NULL;
}

const struct zn_snode_interface impl = {
    .get_texture = get_texture,
};

TEST(general)
{
  struct zn_snode *root = zn_snode_create(NULL, &impl);

  struct zn_snode *node1 = zn_snode_create(NULL, &impl);
  struct zn_snode *node11 = zn_snode_create(NULL, &impl);
  struct zn_snode *node12 = zn_snode_create(NULL, &impl);
  struct zn_snode *node121 = zn_snode_create(NULL, &impl);

  struct zn_snode *node2 = zn_snode_create(NULL, &impl);

  zn_snode_set_position(node1, root, 10.5F, 20.5F);
  zn_snode_set_position(node11, node1, -4.3F, 3.3F);
  zn_snode_set_position(node12, node1, 10.3F, -9.9F);
  zn_snode_set_position(node121, node12, 9.5F, 3.5F);

  zn_snode_set_position(node2, root, 9.9F, 3.4F);

  struct wlr_fbox box11;
  struct wlr_fbox box121;
  zn_snode_get_fbox(node11, &box11);
  zn_snode_get_fbox(node121, &box121);

  ASSERT_EQUAL_DOUBLE(10.5F - 4.3F, box11.x);
  ASSERT_EQUAL_DOUBLE(20.5F + 3.3F, box11.y);

  ASSERT_EQUAL_DOUBLE(10.5F + 10.3F + 9.5F, box121.x);
  ASSERT_EQUAL_DOUBLE(20.5F - 9.9F + 3.5F, box121.y);

  zn_snode_set_position(node1, root, 11.6F, -20.6F);

  zn_snode_get_fbox(node11, &box11);
  zn_snode_get_fbox(node121, &box121);

  ASSERT_EQUAL_DOUBLE(11.6F - 4.3F, box11.x);
  ASSERT_EQUAL_DOUBLE(-20.6F + 3.3F, box11.y);

  ASSERT_EQUAL_DOUBLE(11.6F + 10.3F + 9.5F, box121.x);
  ASSERT_EQUAL_DOUBLE(-20.6F - 9.9F + 3.5F, box121.y);
}

TEST(parent_change)
{
  struct zn_snode *root = zn_snode_create(NULL, &impl);

  struct zn_snode *node1 = zn_snode_create(NULL, &impl);
  struct zn_snode *node2 = zn_snode_create(NULL, &impl);
  struct zn_snode *node3 = zn_snode_create(NULL, &impl);
  struct zn_snode *node4 = zn_snode_create(NULL, &impl);

  zn_snode_set_position(node4, node3, 4, 0.4F);
  zn_snode_set_position(node3, node2, 3, 0.3F);
  zn_snode_set_position(node2, node1, 2, 0.2F);
  zn_snode_set_position(node1, root, 1, 0.1F);

  struct wlr_fbox box4;
  zn_snode_get_fbox(node4, &box4);

  ASSERT_EQUAL_DOUBLE(1 + 2 + 3 + 4, box4.x);
  ASSERT_EQUAL_DOUBLE(0.1F + 0.2F + 0.3F + 0.4F, box4.y);

  zn_snode_set_position(node2, root, 12, 1.2F);
  zn_snode_get_fbox(node4, &box4);

  ASSERT_EQUAL_DOUBLE(12 + 3 + 4, box4.x);
  ASSERT_EQUAL_DOUBLE(1.2F + 0.3F + 0.4F, box4.y);

  zn_snode_set_position(node2, NULL, 0.0F, 0.0F);
  zn_snode_get_fbox(node4, &box4);

  ASSERT_EQUAL_DOUBLE(3 + 4, box4.x);
  ASSERT_EQUAL_DOUBLE(0.3F + 0.4F, box4.y);
}
