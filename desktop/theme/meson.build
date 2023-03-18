_zen_desktop_theme_srcs = [
  'src/color.c',
  'src/drop-shadow.c',
  'src/icon.c',
  'src/theme.c',
]

_zen_desktop_theme_inc = include_directories('include')

_zen_desktop_theme_private_inc = [
  include_directories('src'),
  _zen_desktop_theme_inc,
]

_zen_desktop_theme_pub_deps = [
  cairo_dep,
  rsvg_dep,
  wlroots_dep,
  zen_common_dep,
]

_zen_desktop_theme_deps = [
] + _zen_desktop_theme_pub_deps

_zen_desktop_theme_lib = static_library(
  'zen-desktop-theme',
  _zen_desktop_theme_srcs,
  install: false,
  c_args: ['-include', 'constants.h'],
  dependencies: _zen_desktop_theme_deps,
  include_directories: _zen_desktop_theme_inc,
)

zen_desktop_theme_dep = declare_dependency(
  link_with: _zen_desktop_theme_lib,
  include_directories: _zen_desktop_theme_inc,
  dependencies: _zen_desktop_theme_pub_deps,
)

if get_option('tests')
  subdir('tests')
endif
