_zen_desktop_theme_srcs = [
  'src/color.c',
  'src/drop-shadow.c',
  'src/theme.c',
]

_zen_desktop_theme_inc = include_directories('include')

_zen_desktop_theme_private_inc = [
  include_directories('src'),
  _zen_desktop_theme_inc,
]

_zen_desktop_theme_deps = [
  cairo_dep,
  zen_common_dep,
]

_zen_desktop_theme_lib = static_library(
  'zen-desktop-theme',
  _zen_desktop_theme_srcs,
  install: false,
  dependencies: _zen_desktop_theme_deps,
  include_directories: _zen_desktop_theme_inc,
)

zen_desktop_theme_dep = declare_dependency(
  link_with: _zen_desktop_theme_lib,
  include_directories: _zen_desktop_theme_inc,
)

if get_option('tests')
  subdir('tests')
endif
