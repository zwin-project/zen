#include "zen-common/types.h"

#include <string.h>

int
zn_array_to_off_t(struct wl_array *array, off_t *value)
{
  if (array->size == sizeof(int32_t)) {
    int32_t v;
    memcpy(&v, array->data, array->size);
    *value = v;
  } else if (array->size == sizeof(int64_t)) {
    int64_t v;
    memcpy(&v, array->data, array->size);
    *value = v;
  } else {
    return -1;
  }

  return 0;
}

int
zn_off_t_to_array(off_t value, struct wl_array *array)
{
  wl_array_init(array);

  off_t *container = wl_array_add(array, sizeof(off_t));
  if (container == NULL) return -1;

  *container = value;
  return 0;
}

int
zn_array_to_uint64_t(struct wl_array *array, uint64_t *value)
{
  if (array->size != sizeof(uint64_t)) return -1;

  memcpy(value, array->data, array->size);

  return 0;
}

int
zn_uint64_t_to_array(uint64_t value, struct wl_array *array)
{
  wl_array_init(array);

  off_t *container = wl_array_add(array, sizeof(uint64_t));
  if (container == NULL) return -1;

  *container = value;
  return 0;
}

int
zn_array_to_vec3(struct wl_array *array, vec3 vec)
{
  if (sizeof(vec3) != array->size) return -1;

  memcpy(vec, array->data, sizeof(vec3));

  return 0;
}

int
zn_vec3_to_array(vec3 vec, struct wl_array *array)
{
  wl_array_init(array);

  float *container = wl_array_add(array, sizeof(vec3));
  if (container == NULL) return -1;

  memcpy(container, vec, sizeof(vec3));

  return 0;
}

int
zn_array_to_versor(struct wl_array *array, versor vec)
{
  if (sizeof(versor) != array->size) return -1;

  memcpy(vec, array->data, sizeof(versor));

  return 0;
}

int
zn_array_to_float(struct wl_array *array, float *value)
{
  if (sizeof(float) != array->size) return -1;

  memcpy(value, array->data, sizeof(float));

  return 0;
}
