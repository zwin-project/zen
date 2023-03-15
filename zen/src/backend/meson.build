_zen_backend_srcs = [
  'backend.c',
  'compositor.c',
  'layer-surface.c',
  'output.c',
  'pointer.c',
  'screen-backend.c',
  'surface-snode.c',
  'xwayland-surface.c',
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
