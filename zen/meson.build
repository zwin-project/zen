_zen_srcs = [
  'src/backend.c',
  'src/backend/output.c',
  'src/backend/pointer.c',

  'src/config.c',
  'src/screen.c',
  'src/server.c',
]

zen_inc = include_directories('include')
zen_private_inc = [
  include_directories('src'),
  zen_inc,
]

_zen_pub_deps = [
  pixman_dep,
  toml_dep,
  wayland_server_dep,
]

_zen_deps = [
  drm_dep,
  egl_dep,
  gl_dep,
  gbm_dep,
  glew_dep,
  wlroots_dep,
  zen_common_dep,
  zen_wlr_glew_renderer_dep,
] + _zen_pub_deps

_zen_lib = static_library(
  'zen-lib',
  _zen_srcs,
  install: false,
  include_directories: zen_private_inc,
  dependencies: _zen_deps,
)

zen_dep = declare_dependency(
  link_with: _zen_lib,
  include_directories: zen_inc,
  dependencies: _zen_pub_deps,
)

if get_option('tests')
  subdir('tests')
endif
