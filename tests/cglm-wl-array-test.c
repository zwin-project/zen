#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>

#include "test-runner.h"

TEST(glm_vec3_to_wl_array)
{
  struct wl_array array;
  vec3 v;
  float *data;

  wl_array_init(&array);

  data = wl_array_add(&array, sizeof(float));
  *data = 1.0f;
  data = wl_array_add(&array, sizeof(float));
  *data = 2.0f;
  data = wl_array_add(&array, sizeof(float));
  *data = 9.9f;

  assert(glm_vec3_from_wl_array(v, &array) == 0);

  assert(v[0] == 1.0f);
  assert(v[1] == 2.0f);
  assert(v[2] == 9.9f);

  data = wl_array_add(&array, sizeof(float));
  assert(glm_vec3_from_wl_array(v, &array) != 0);
}

TEST(glm_vec3_from_wl_array)
{
  struct wl_array array;
  float *data;
  vec3 v = {1.0f, 3.3f, -3.4f};

  wl_array_init(&array);

  glm_vec3_to_wl_array(v, &array);

  data = array.data;

  assert(data[0] == 1.0f);
  assert(data[1] == 3.3f);
  assert(data[2] == -3.4f);
}
