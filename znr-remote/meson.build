_znr_remote_inc = include_directories('include')

_znr_remote_srcs = [
  'src/loop.cc',
  'src/peer.cc',
  'src/remote.cc',
  'src/session.cc',
  'src/virtual-object.cc',
]

_znr_remote_deps = [
  wayland_server_dep,
  zen_common_dep,
  zen_remote_server_dep,
]

_znr_remote_lib = static_library(
  'znr',
  _znr_remote_srcs,
  dependencies: _znr_remote_deps,
  include_directories: [zen_inc, _znr_remote_inc],
  install: false,
)

znr_remote_dep = declare_dependency(
  link_with: _znr_remote_lib,
  include_directories: _znr_remote_inc,
)
