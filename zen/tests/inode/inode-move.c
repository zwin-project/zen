#include <cglm/quat.h>
#include <cglm/vec3.h>
#include <string.h>

#include "inode.h"
#include "test-harness.h"

TEST(general)
{
  struct zn_inode *root = zn_inode_create(NULL, &zn_inode_noop_implementation);

  struct zn_inode *n1 = zn_inode_create(NULL, &zn_inode_noop_implementation);
  struct zn_inode *n2 = zn_inode_create(NULL, &zn_inode_noop_implementation);

  versor x90;
  versor y90;
  versor z90;

  glm_quat(x90, GLM_PI_2f, 1, 0, 0);
  glm_quat(y90, GLM_PI_2f, 0, 1, 0);
  glm_quat(z90, GLM_PI_2f, 0, 0, 1);

  zn_inode_move(root, NULL, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);
  zn_inode_move(n1, root, (vec3){20, 30, 40}, y90);
  zn_inode_move(n2, n1, (vec3){30, 40, 50}, z90);

  vec3 local = {1, 0, 0};
  vec3 global;

  glm_mat4_mulv3(n1->transform_abs, local, 1, global);

  ASSERT_EQUAL_VEC3(((vec3){20, 30, 39}), global);

  zn_inode_move(root, NULL, (vec3){10, 20, 30}, x90);

  glm_mat4_mulv3(n1->transform_abs, local, 1, global);

  ASSERT_EQUAL_VEC3(((vec3){30, -19, 60}), global);

  glm_mat4_mulv3(n2->transform_abs, local, 1, global);

  ASSERT_EQUAL_VEC3(((vec3){80, 10, 101}), global);
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
  struct zn_inode *root2 = zn_inode_create(NULL, &zn_inode_noop_implementation);
  struct zn_inode *node = zn_inode_create(&data, &implementation);

  zn_inode_map(root);
  zn_inode_set_xr_system(root, xr_system);

  zn_inode_move(node, root, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);

  ASSERT_EQUAL_BOOL(true, data.mapped);
  ASSERT_EQUAL_BOOL(true, data.activated);
  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(node));
  ASSERT_EQUAL_BOOL(true, zn_inode_is_active(node));

  zn_inode_move(node, root2, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);

  ASSERT_EQUAL_BOOL(true, data.unmapped);
  ASSERT_EQUAL_BOOL(true, data.deactivated);
  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(node));
  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(node));
}

TEST(map)
{
  struct zn_inode *root = zn_inode_create(NULL, &zn_inode_noop_implementation);
  struct zn_inode *root2 = zn_inode_create(NULL, &zn_inode_noop_implementation);
  struct zn_inode *node = zn_inode_create(NULL, &zn_inode_noop_implementation);

  zn_inode_map(root);

  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(node));

  zn_inode_move(node, root, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);

  ASSERT_EQUAL_BOOL(true, zn_inode_is_mapped(node));

  zn_inode_move(node, root2, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);

  ASSERT_EQUAL_BOOL(false, zn_inode_is_mapped(node));
}

TEST(active)
{
  struct zn_inode *root = zn_inode_create(NULL, &zn_inode_noop_implementation);
  struct zn_inode *root2 = zn_inode_create(NULL, &zn_inode_noop_implementation);
  struct zn_inode *node = zn_inode_create(NULL, &zn_inode_noop_implementation);
  struct zn_xr_system *xr_system = (void *)1;

  zn_inode_set_xr_system(root, xr_system);

  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(node));

  zn_inode_move(node, root, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);

  ASSERT_EQUAL_BOOL(true, zn_inode_is_active(node));

  zn_inode_move(node, root2, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);

  ASSERT_EQUAL_BOOL(false, zn_inode_is_active(node));
}
