_zen_openvr_immersive_backend_srcs =[
  'immersive-backend.cc',
]

_zen_openvr_immersive_backend_deps =[
  zen_common_dep,
]

_zen_openvr_immersive_backend_lib = static_library(
  'zen-openvr-immersive-backend',
  _zen_openvr_immersive_backend_srcs,
  dependencies: _zen_openvr_immersive_backend_deps,
  include_directories: _zen_immersive_backend_inc,
  install: false,
)

zen_openvr_immersive_backend_dep = declare_dependency(
  link_with: _zen_openvr_immersive_backend_lib,
  include_directories: _zen_immersive_backend_inc,
)
