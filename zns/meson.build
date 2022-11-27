_zen_shell_inc = include_directories('.')

_zen_shell_srcs = [
  'bounded.c',
  'shell.c',

  'ray-grab/default.c',
  'ray-grab/move.c',
]

_zen_shell_deps = [
  zen_dep,
]

_zen_shell_lib = static_library(
  'zen-shell',
  _zen_shell_srcs,
  include_directories: [zen_inc, _zen_shell_inc],
  dependencies: _zen_shell_deps,
  install: false,
)

zen_shell_dep = declare_dependency(
  link_with: _zen_shell_lib,
)
