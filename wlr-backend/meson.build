_zen_wlr_backend_srcs = [
  'src/backend.c',
]

_zen_wlr_backend_deps = [
  zen_common_dep,
]

_zen_wlr_backend_inc = include_directories('include')

_zen_wlr_backend_lib = static_library(
  'zen-wlr-backend-lib',
  _zen_wlr_backend_srcs,
  install: false,
  include_directories: [_zen_wlr_backend_inc, zen_inc],
  dependencies: _zen_wlr_backend_deps,
)

zen_wlr_backend_dep = declare_dependency(
  link_with: _zen_wlr_backend_lib,
)
