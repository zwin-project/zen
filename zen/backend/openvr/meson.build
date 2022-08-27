_zen_openvr_immersive_backend_srcs =[
  'immersive-backend.cc',
  'vr-system.cc',
]

_zen_openvr_immersive_backend_deps =[
  openvr_dep,
  wayland_server_dep,
  zen_common_dep,
]

_zen_openvr_immersive_backend_lib = static_library(
  'zen-openvr-immersive-backend',
  _zen_openvr_immersive_backend_srcs,
  dependencies: _zen_openvr_immersive_backend_deps,
  include_directories: zen_inc,
  install: false,
)

zen_openvr_immersive_backend_dep = declare_dependency(
  link_with: _zen_openvr_immersive_backend_lib,
)
