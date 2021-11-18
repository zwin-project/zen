deps_oepngl_renderer = [
  dep_libzen_private,
  dep_glew,
]

srcs_opengl_renderer = [
  'opengl-renderer.c',
]

lib_opengl_renderer = static_library(
  'opengl-renderer',
  srcs_opengl_renderer,
  install: false,
  dependencies: deps_oepngl_renderer,
)

dep_oepngl_renderer_public = declare_dependency(
  link_with: lib_opengl_renderer,
  include_directories: public_inc,
)
