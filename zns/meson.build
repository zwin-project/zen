zen_shell_inc = include_directories('include')

_zen_shell_srcs = [
  'src/board.c',
  'src/bounded.c',
  'src/expansive.c',
  'src/node.c',
  'src/seat-capsule.c',
  'src/shell.c',

  'src/ray-grab/board-move.c',
  'src/ray-grab/default.c',
  'src/ray-grab/down.c',
  'src/ray-grab/move.c',
]

_zen_shell_deps = [
  zen_dep,
]

_zen_shell_lib = static_library(
  'zen-shell',
  _zen_shell_srcs,
  include_directories: [zen_inc, zen_shell_inc],
  dependencies: _zen_shell_deps,
  install: false,
)

zen_shell_dep = declare_dependency(
  link_with: _zen_shell_lib,
)
