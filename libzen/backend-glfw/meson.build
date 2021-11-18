deps_glfw_backend = [
  dep_oepngl_renderer_public,
  dep_libzen_private,
  dep_glfw3,
  dep_glew,
]

srcs_glfw_backend = [
  'backend.c',
  'output.c',
]

lib_glfw_backend = static_library(
  'glfw-backend',
  srcs_glfw_backend,
  install: false,
  dependencies: deps_glfw_backend,
)

dep_glfw_backend_public = declare_dependency(
  link_with: lib_glfw_backend,
  include_directories: public_inc,
)
