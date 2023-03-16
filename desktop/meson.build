_zen_desktop_srcs = [
  'src/cursor-grab.c',
  'src/cursor-grab/default.c',
  'src/cursor-grab/down.c',
  'src/cursor-grab/move.c',
  'src/cursor-grab/resize.c',

  'src/screen.c',
  'src/screen-layout.c',
  'src/shell.c',

  'src/ui/decoration.c',
  'src/ui/header-bar.c',

  'src/view.c',
]

_zen_desktop_inc = include_directories('include')
_zen_desktop_private_inc = [
  include_directories('src'),
  _zen_desktop_inc,
]

_zen_desktop_deps = [
  cairo_dep,
  rsvg_dep,
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

if get_option('tests')
  subdir('tests')
endif
