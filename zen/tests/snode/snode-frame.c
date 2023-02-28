#include <cglm/types.h>
#include <wlr/render/wlr_texture.h>

#include "mock/output.h"
#include "test-harness.h"
#include "zen-common/util.h"
#include "zen/snode.h"

static struct wlr_texture *
test_get_texture(void *user_data UNUSED)
{
  return NULL;
}

static void
test_frame(void *user_data, const struct timespec *when UNUSED)
{
  int *counter = user_data;
  (*counter)++;
}

const struct zn_snode_interface test_impl = {
    .get_texture = test_get_texture,
    .frame = test_frame,
};

TEST(frame)
{
  struct timespec now;
  struct zn_mock_output *output = zn_mock_output_create(0, 0);

  int counter = 0;

  struct zn_snode *node1 = zn_snode_create(&counter, &test_impl);
  struct zn_snode *node2 = zn_snode_create(&counter, &test_impl);
  struct zn_snode *node3 = zn_snode_create(&counter, &test_impl);
  struct zn_snode *node4 = zn_snode_create(&counter, &test_impl);

  zn_snode_set_position(node1, output->screen->snode_root, (vec2){0, 0});
  zn_snode_set_position(node2, node1, (vec2){0, 0});
  zn_snode_set_position(node3, node1, (vec2){0, 0});
  zn_snode_set_position(node4, node2, (vec2){0, 0});

  zn_screen_notify_frame(output->screen, &now);

  ASSERT_EQUAL_INT(4, counter);

  zn_snode_set_position(node2, NULL, (vec2){0, 0});

  zn_screen_notify_frame(output->screen, &now);

  ASSERT_EQUAL_INT(6, counter);
}