deps_oepngl_renderer = [
  dep_cglm,
  dep_glew,
  dep_libzen_compositor_public,
]

srcs_opengl_renderer = [
  'opengl.c',
  'opengl-component.c',
  'opengl-renderer.c',
  zigen_opengl_protocol_c,
  zigen_opengl_server_protocol_h,
]

lib_opengl_renderer = static_library(
  'zen-opengl-renderer',
  srcs_opengl_renderer,
  install: false,
  dependencies: deps_oepngl_renderer,
)

dep_oepngl_renderer_public = declare_dependency(
  link_with: lib_opengl_renderer,
  include_directories: public_inc,
)
