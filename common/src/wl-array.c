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
zn_wl_array_from_vec3(struct wl_array *array, vec3 value)
{
  void *data = wl_array_add(array, sizeof(vec3));
  if (data == NULL) {
    return false;
  }

  memcpy(data, value, sizeof(vec3));

  return true;
}

bool
zn_wl_array_to_vec3(struct wl_array *array, vec3 value)
{
  if (array->size != sizeof(vec3)) {
    return false;
  }

  memcpy(value, array->data, array->size);

  return true;
}
