#include <libzen-compositor/libzen-compositor.h>
#include <string.h>

static const size_t vec3_size = sizeof(vec3);
static const size_t versor_size = sizeof(versor);

WL_EXPORT int
glm_vec3_from_wl_array(vec3 v, struct wl_array* array)
{
  float* data = array->data;
  if (array->size != vec3_size) return -1;
  memcpy(v, data, vec3_size);
  return 0;
}

WL_EXPORT void
glm_vec3_to_wl_array(vec3 v, struct wl_array* array)
{
  if (array->alloc > 0) {
    wl_array_release(array);
    wl_array_init(array);
  }
  float* data = wl_array_add(array, vec3_size);
  memcpy(data, v, vec3_size);
}

WL_EXPORT int
glm_versor_from_wl_array(versor v, struct wl_array* array)
{
  float* data = array->data;
  if (array->size != versor_size) return -1;
  memcpy(v, data, versor_size);
  glm_quat_normalize(v);
  return 0;
}

WL_EXPORT void
glm_versor_to_wl_array(versor v, struct wl_array* array)
{
  if (array->alloc > 0) {
    wl_array_release(array);
    wl_array_init(array);
  }
  float* data = wl_array_add(array, versor_size);
  memcpy(data, v, versor_size);
}
