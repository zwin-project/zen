_zen_backend_srcs = [
  'default-backend.c',

  'screen/compositor.c',
  'screen/keyboard.c',
  'screen/layer-shell.c',
  'screen/layer-surface.c',
  'screen/output.c',
  'screen/pointer.c',
  'screen/surface-snode.c',
  'screen/xwayland-surface.c',
]

_zen_backend_lib = static_library(
  'zen-backend',
  _zen_backend_srcs,
  install: false,
  include_directories: zen_private_inc,
  dependencies: _zen_deps,
)

zen_backend_dep = declare_dependency(
  link_with: _zen_backend_lib,
)
