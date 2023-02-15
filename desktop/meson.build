_zen_desktop_srcs = [
  'src/shell.c',
]

_zen_desktop_inc = include_directories('include')

_zen_desktop_deps = [
  zen_common_dep,
  zen_dep,
]

_zen_desktop_lib = static_library(
  'zen-desktop-lib',
  _zen_desktop_srcs,
  install: false,
  dependencies: _zen_desktop_deps,
  include_directories: _zen_desktop_inc,
)

zen_desktop_dep = declare_dependency(
  link_with: _zen_desktop_lib,
  include_directories: _zen_desktop_inc,
)
