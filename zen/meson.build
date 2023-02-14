_zen_srcs = [
  'src/config.c',
  'src/screen.c',
  'src/server.c',
]

zen_inc = include_directories('include')
zen_src_inc = include_directories('src')

_zen_pub_deps = [
  pixman_dep,
  toml_dep,
  wayland_server_dep,
]

zen_deps = [
  zen_common_dep,
] + _zen_pub_deps

_zen_lib = static_library(
  'zen-lib',
  _zen_srcs,
  install: false,
  include_directories: zen_inc,
  dependencies: zen_deps,
)

zen_dep = declare_dependency(
  link_with: _zen_lib,
  include_directories: zen_inc,
  dependencies: _zen_pub_deps,
)

if get_option('tests')
  subdir('tests')
endif
