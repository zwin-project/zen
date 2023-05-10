#include <cglm/quat.h>
#include <cglm/vec3.h>

#include "test-harness.h"
#include "zen/inode.h"

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
