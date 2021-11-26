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
  float *data = (float *)wl_array_add(array, sizeof(float) * 3);
  data[0] = v.x;
  data[1] = v.y;
  data[2] = v.z;
}

}  // namespace zukou
