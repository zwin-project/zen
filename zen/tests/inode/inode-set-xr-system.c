#include <cglm/quat.h>
#include <cglm/vec3.h>
#include <string.h>

#include "inode.h"
#include "test-harness.h"

TEST(map)
{
  struct zn_xr_system *xr_system = (void *)1;

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

  zn_inode_set_xr_system(root, xr_system);

  ASSERT_EQUAL_BOOL(true, zn_inode_is_active(root));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_active(n1));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_active(n2));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_active(n3));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_active(n11));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_active(n12));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_active(n21));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_active(n211));

  zn_inode_set_xr_system(root, NULL);

  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(root));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(n1));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(n2));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(n3));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(n11));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(n12));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(n21));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(n211));

  zn_inode_set_xr_system(root, xr_system);

  ASSERT_EQUAL_BOOL(true, zn_inode_is_active(root));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_active(n1));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_active(n2));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_active(n3));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_active(n11));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_active(n12));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_active(n21));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_active(n211));

  zn_inode_move(n1, NULL, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);
  zn_inode_move(n2, NULL, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);

  ASSERT_EQUAL_BOOL(true, zn_inode_is_active(root));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(n1));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(n2));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_active(n3));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(n11));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(n12));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(n21));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(n211));

  zn_inode_set_xr_system(root, NULL);
  zn_inode_move(n1, root, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);

  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(root));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(n1));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(n2));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(n3));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(n11));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(n12));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(n21));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(n211));

  zn_inode_set_xr_system(root, xr_system);

  ASSERT_EQUAL_BOOL(true, zn_inode_is_active(root));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_active(n1));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(n2));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_active(n3));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_active(n11));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_active(n12));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(n21));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(n211));
}

struct test_data {
  bool mapped;
  bool unmapped;
  bool activated;
  bool deactivated;
  bool moved;
};

static void
mapped(void *impl_data UNUSED)
{
  struct test_data *data = impl_data;
  data->mapped = true;
}

static void
unmapped(void *impl_data UNUSED)
{
  struct test_data *data = impl_data;
  data->unmapped = true;
}

static void
activated(void *impl_data UNUSED)
{
  struct test_data *data = impl_data;
  data->activated = true;
}

static void
deactivated(void *impl_data UNUSED)
{
  struct test_data *data = impl_data;
  data->deactivated = true;
}

static void
moved(void *impl_data UNUSED)
{
  struct test_data *data = impl_data;
  data->moved = true;
}

TEST(signal)
{
  static const struct zn_inode_interface implementation = {
      .mapped = mapped,
      .unmapped = unmapped,
      .activated = activated,
      .deactivated = deactivated,
      .moved = moved,
  };

  struct test_data data = {0};

  struct zn_xr_system *xr_system = (void *)1;

  struct zn_inode *root = zn_inode_create(NULL, &zn_inode_noop_implementation);
  struct zn_inode *node = zn_inode_create(&data, &implementation);

  zn_inode_move(node, root, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);

  ASSERT_EQUAL_BOOL(true, data.moved);

  ASSERT_EQUAL_BOOL(false, data.activated);
  zn_inode_set_xr_system(root, xr_system);
  ASSERT_EQUAL_BOOL(true, data.activated);

  ASSERT_EQUAL_BOOL(false, data.deactivated);
  zn_inode_set_xr_system(root, NULL);
  ASSERT_EQUAL_BOOL(true, data.deactivated);
}

FAIL_TEST(non_root_map)
{
  struct zn_xr_system *xr_system = (void *)1;

  struct zn_inode *root = zn_inode_create(NULL, &zn_inode_noop_implementation);
  struct zn_inode *node = zn_inode_create(NULL, &zn_inode_noop_implementation);

  zn_inode_move(node, root, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);

  zn_inode_set_xr_system(node, xr_system);
}

TEST(destroy)
{
  struct zn_xr_system *xr_system = (void *)1;

  struct zn_inode *root = zn_inode_create(NULL, &zn_inode_noop_implementation);
  struct zn_inode *node = zn_inode_create(NULL, &zn_inode_noop_implementation);

  zn_inode_move(node, root, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);

  zn_inode_map(root);
  zn_inode_set_xr_system(root, xr_system);

  ASSERT_EQUAL_BOOL(true, zn_inode_is_active(node));
}
