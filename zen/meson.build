_zen_srcs = [
  'src/screen.c',
  'src/server.c',
]

zen_inc = include_directories('include')

_zen_deps = [
  wayland_server_dep,
  zen_common_dep,
]

_zen_lib = static_library(
  'zen-lib',
  _zen_srcs,
  install: false,
  include_directories: zen_inc,
  dependencies: _zen_deps,
)

zen_dep = declare_dependency(
  link_with: _zen_lib,
  include_directories: zen_inc,
)
