_zigzag_inc = include_directories('include')

_zigzag_srcs = [
  'src/cairo_util.c',
  'src/layout.c',
  'src/node.c',
]

_zigzag_deps = [
  cairo_dep,
  wayland_server_dep,
  wlroots_dep,
  zen_common_dep,
]

_zigzag_lib = static_library(
  'zigzag',
  _zigzag_srcs,
  include_directories: _zigzag_inc,
  dependencies: _zigzag_deps,
  install: false,
)

zigzag_dep = declare_dependency(
  link_with: _zigzag_lib,
  include_directories: _zigzag_inc,
)
