_zen_wlr_glew_renderer_srcs = [
  'src/egl.c',
  'src/pixel_format.c',
  'src/glew.c',
  'src/shaders.c',
  'src/texture.c',
]

_zen_wlr_glew_renderer_inc = include_directories('include')

_zen_wlr_glew_renderer_deps = [
  wlroots_dep,
  zen_common_dep,
]

_zen_wlr_glew_renderer_lib = static_library(
  'zen-wlr-glew-renderer',
  _zen_wlr_glew_renderer_srcs,
  install: false,
  dependencies: _zen_wlr_glew_renderer_deps,
  include_directories: _zen_wlr_glew_renderer_inc,
)

zen_wlr_glew_renderer_dep = declare_dependency(
  link_with: _zen_wlr_glew_renderer_lib,
  include_directories: _zen_wlr_glew_renderer_inc,
)
