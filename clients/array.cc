#include "array.h"

#include <cstring>

namespace zen::client {

bool
to_vec3(struct wl_array *array, glm::vec3 &vec)
{
  if (array->size != sizeof(glm::vec3)) return false;

  std::memcpy(&vec, array->data, sizeof(glm::vec3));

  return true;
}

bool
to_array(glm::vec3 &vec, struct wl_array *array)
{
  wl_array_init(array);

  void *container = wl_array_add(array, sizeof(glm::vec3));
  if (container == NULL) return false;

  std::memcpy(container, &vec, sizeof(glm::vec3));

  return true;
}

}  // namespace zen::client
