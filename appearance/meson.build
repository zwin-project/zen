_zen_appearance_srcs = [
  'appearance.c',
  'scene/virtual-object.c',
  'scene/virtual-object/rendering-unit.c',
]

_zen_appearance_deps = [
  wayland_server_dep,
  zen_common_dep,
  zgnr_dep,
]

_zen_appearance_lib = static_library(
  'appearance',
  _zen_appearance_srcs,
  include_directories: zen_inc,
  dependencies: _zen_appearance_deps,
  install: false,
)

zen_appearance_dep = declare_dependency(
  link_with: _zen_appearance_lib,
)
