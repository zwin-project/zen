#include "zen-common/wl-array.h"

#include <string.h>

bool
zn_wl_array_to_int64_t(struct wl_array *array, int64_t *value)
{
  if (array->size != sizeof(*value)) {
    return false;
  }

  memcpy(value, array->data, array->size);

  return true;
}

bool
zn_wl_array_to_uint64_t(struct wl_array *array, uint64_t *value)
{
  if (array->size != sizeof(*value)) {
    return false;
  }

  memcpy(value, array->data, array->size);

  return true;
}

bool
zn_wl_array_from_vec3(struct wl_array *array, vec3 v)
{
  void *data = wl_array_add(array, sizeof(vec3));
  if (data == NULL) {
    return false;
  }

  memcpy(data, v, sizeof(vec3));

  return true;
}
