_zen_remote_immersive_backend_srcs = [
  'loop.cc',
  'xr-system.cc',
  'xr-system-manager.cc',
]

_zen_remote_immersive_backend_deps = [
  _zen_deps,
  zen_remote_server_dep,
]

_zen_remote_immersive_backend_lib = static_library(
  'zen-remote-immersive-backend',
  _zen_remote_immersive_backend_srcs,
  install: false,
  cpp_args: [ '-include', 'pch/pch.h' ],
  include_directories: zen_private_inc,
  dependencies: _zen_remote_immersive_backend_deps,
  cpp_pch: 'pch/pch.h',
)

zen_remote_immersive_backend_dep = declare_dependency(
  link_with: _zen_remote_immersive_backend_lib,
)
