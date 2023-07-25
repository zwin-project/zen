#include <cglm/quat.h>
#include <cglm/vec3.h>

#include "inode.h"
#include "test-harness.h"

TEST(map)
{
  struct zn_inode *root = zn_inode_create(NULL, &zn_inode_noop_implementation);

  struct zn_inode *n1 = zn_inode_create(NULL, &zn_inode_noop_implementation);
  struct zn_inode *n2 = zn_inode_create(NULL, &zn_inode_noop_implementation);
  struct zn_inode *n3 = zn_inode_create(NULL, &zn_inode_noop_implementation);
  struct zn_inode *n11 = zn_inode_create(NULL, &zn_inode_noop_implementation);
  struct zn_inode *n12 = zn_inode_create(NULL, &zn_inode_noop_implementation);
  struct zn_inode *n21 = zn_inode_create(NULL, &zn_inode_noop_implementation);
  struct zn_inode *n211 = zn_inode_create(NULL, &zn_inode_noop_implementation);

  zn_inode_move(n1, root, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);
  zn_inode_move(n2, root, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);
  zn_inode_move(n3, root, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);
  zn_inode_move(n11, n1, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);
  zn_inode_move(n12, n1, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);
  zn_inode_move(n21, n2, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);
  zn_inode_move(n211, n21, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);

  zn_inode_map(root);

  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(root));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(n1));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(n2));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(n3));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(n11));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(n12));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(n21));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(n211));

  zn_inode_unmap(root);

  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(root));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(n1));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(n2));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(n3));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(n11));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(n12));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(n21));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(n211));

  zn_inode_map(root);

  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(root));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(n1));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(n2));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(n3));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(n11));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(n12));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(n21));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(n211));

  zn_inode_move(n1, NULL, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);
  zn_inode_move(n2, NULL, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);

  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(root));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(n1));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(n2));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(n3));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(n11));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(n12));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(n21));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(n211));

  zn_inode_unmap(root);
  zn_inode_move(n1, root, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);

  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(root));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(n1));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(n2));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(n3));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(n11));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(n12));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(n21));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(n211));

  zn_inode_map(root);

  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(root));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(n1));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(n2));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(n3));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(n11));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(n12));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(n21));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(n211));

  zn_inode_move(n2, root, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);

  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(root));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(n1));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(n2));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(n3));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(n11));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(n12));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(n21));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(n211));
}

FAIL_TEST(non_root_map)
{
  struct zn_inode *root = zn_inode_create(NULL, &zn_inode_noop_implementation);
  struct zn_inode *node = zn_inode_create(NULL, &zn_inode_noop_implementation);

  zn_inode_move(node, root, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);

  zn_inode_map(node);
}

FAIL_TEST(non_root_unmap)
{
  struct zn_inode *root = zn_inode_create(NULL, &zn_inode_noop_implementation);
  struct zn_inode *node = zn_inode_create(NULL, &zn_inode_noop_implementation);

  zn_inode_move(node, root, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);

  zn_inode_unmap(node);
}

TEST(already_mapped)
{
  struct zn_inode *root = zn_inode_create(NULL, &zn_inode_noop_implementation);
  struct zn_inode *node = zn_inode_create(NULL, &zn_inode_noop_implementation);

  zn_inode_move(node, root, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);

  zn_inode_map(root);
  zn_inode_map(root);  // no error
}

TEST(destroy)
{
  struct zn_inode *root = zn_inode_create(NULL, &zn_inode_noop_implementation);
  struct zn_inode *node = zn_inode_create(NULL, &zn_inode_noop_implementation);

  zn_inode_move(node, root, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);

  zn_inode_map(root);

  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(node));

  zn_inode_destroy(root);

  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(node));
  ASSERT_EQUAL_POINTER(NULL, node->parent);
}
