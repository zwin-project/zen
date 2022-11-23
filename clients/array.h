#pragma once

#include <wayland-server-core.h>

#include <glm/vec3.hpp>

namespace zen::client {

bool to_vec3(struct wl_array *array, glm::vec3 &vec);

bool to_array(glm::vec3 &vec, struct wl_array *array);

}  // namespace zen::client
