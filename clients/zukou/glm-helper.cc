#include <string.h>
#include <zukou.h>

#include <glm/glm.hpp>

namespace zukou {

glm::vec3
glm_vec3_from_wl_array(struct wl_array *array)
{
  float *data = (float *)array->data;
  return glm::vec3(data[0], data[1], data[2]);
}

void
glm_vec3_to_wl_array(glm::vec3 v, struct wl_array *array)
{
  size_t size = sizeof(float) * 3;
  float *data = (float *)wl_array_add(array, size);
  memcpy(data, &v, size);
}

void
glm_vec4_to_wl_array(glm::vec4 v, struct wl_array *array)
{
  size_t size = sizeof(float) * 4;
  float *data = (float *)wl_array_add(array, size);
  memcpy(data, &v, size);
}

void
glm_mat4_to_wl_array(glm::mat4 m, struct wl_array *array)
{
  size_t size = sizeof(float) * 16;
  float *data = (float *)wl_array_add(array, size);
  memcpy(data, &m, size);
}

}  // namespace zukou
