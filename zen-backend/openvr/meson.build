deps_openvr_backend = [
  dep_glew,
  dep_glfw3,
  dep_libzen_compositor_public,
  dep_oepngl_renderer_public,
  dep_openvr,
]

srcs_openvr_backend = [
  'backend.cc',
  'gl-window.cc',
  'hmd.cc',
  'output.cc',
]

lib_openvr_backend = static_library(
  'zen-openvr-backend',
  srcs_openvr_backend,
  install: false,
  dependencies: deps_openvr_backend,
)

dep_openvr_backend_public = declare_dependency(
  link_with: lib_openvr_backend,
  include_directories: public_inc,
)
