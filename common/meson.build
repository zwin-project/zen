_zen_common_srcs = [
  'src/log.c',
  'src/signal.c',
  'src/terminate.c',
  'src/wlr/box.c',
]

_zen_common_inc = include_directories('include')

_zen_common_deps = [
]

_zen_common_lib = static_library(
  'zen-common',
  _zen_common_srcs,
  install: false,
  dependencies: _zen_common_deps,
  include_directories: _zen_common_inc,
)

zen_common_dep = declare_dependency(
  link_with: _zen_common_lib,
  include_directories: _zen_common_inc,
)
