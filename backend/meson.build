_zen_wlr_backend_srcs = [
  'src/backend.c',
  'src/output.c',

  'src/wlr/render/egl.c',
  'src/wlr/render/pixel_format.c',
  'src/wlr/render/renderer.c',
  'src/wlr/render/shaders.c',
  'src/wlr/render/texture.c',
]

_zen_wlr_backend_deps = [
  drm_dep,
  egl_dep,
  gl_dep,
  gbm_dep,
  glew_dep,
  wlroots_dep,
  zen_common_dep,
]

_zen_wlr_backend_inc = include_directories('include')

_zen_wlr_backend_lib = static_library(
  'zen-wlr-backend-lib',
  _zen_wlr_backend_srcs,
  install: false,
  include_directories: [_zen_wlr_backend_inc, zen_inc],
  dependencies: _zen_wlr_backend_deps,
)

zen_wlr_backend_dep = declare_dependency(
  link_with: _zen_wlr_backend_lib,
)
