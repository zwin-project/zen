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
