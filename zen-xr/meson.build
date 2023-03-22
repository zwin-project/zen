_zen_xr_srcs = [
  'src/loop.cc',
  'src/remote-system.cc',
  'src/xr.cc',
]

_zen_xr_public_deps = [
]

_zen_xr_deps = [
  wayland_server_dep,
  zen_common_dep,
  zen_remote_server_dep,
] + _zen_xr_public_deps

_zen_xr_inc = include_directories('include')

_zen_xr_lib = static_library(
  'zen-xr',
  _zen_xr_srcs,
  install: false,
  cpp_args: ['-include', 'pch/pch.h'],
  include_directories: _zen_xr_inc,
  dependencies: _zen_xr_deps,
  cpp_pch: 'pch/pch.h',
)

zen_xr_dep = declare_dependency(
  link_with: _zen_xr_lib,
  include_directories: _zen_xr_inc,
)
