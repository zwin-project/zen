_zen_remote_immersive_backend_srcs =[
  'immersive-backend.cc',
  'log-sink.cc',
  'loop.cc',
]

_zen_remote_immersive_backend_deps =[
  zen_remote_display_system_server_dep,
  wayland_server_dep,
  zen_common_dep,
]

_zen_remote_immersive_backend_lib = static_library(
  'zen-remote-immersive-backend',
  _zen_remote_immersive_backend_srcs,
  dependencies: _zen_remote_immersive_backend_deps,
  include_directories: zen_inc,
  install: false,
)

zen_remote_immersive_backend_dep = declare_dependency(
  link_with: _zen_remote_immersive_backend_lib,
)
