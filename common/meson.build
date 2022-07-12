_zen_common_inc = include_directories('include')

_zen_common_src = [
  'log.c',
]

_zen_common_lib = static_library(
  'zen-common',
  _zen_common_src,
  include_directories: _zen_common_inc,
  install: false,
)

zen_common_dep = declare_dependency(
  link_with: _zen_common_lib,
  include_directories: _zen_common_inc,
)
