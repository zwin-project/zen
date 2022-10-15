_znr_inc = include_directories('include')

_znr_srcs = [
  'src/log.cc',
  'src/loop.cc',
  'src/remote.cc',
  'src/rendering-unit.cc',
]

_znr_deps = [
  wayland_server_dep,
  zen_common_dep,
  zen_remote_server_dep,
]

_znr_lib = static_library(
  'znr',
  _znr_srcs,
  include_directories: _znr_inc,
  dependencies: _znr_deps,
  install: false,
)

znr_dep = declare_dependency(
  link_with: _znr_lib,
  include_directories: _znr_inc,
)

