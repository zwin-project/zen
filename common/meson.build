_zen_common_inc = include_directories('include')

_zen_common_srcs = [
  'cmd.c',
  'font.c',
  'log.c',
  'signal.c',
  'terminate.c',
  'types.c',
  'weak-resource.c',
]

_zen_common_deps = [
  cglm_dep,
  fontconfig_dep,
  wayland_server_dep.partial_dependency(includes: true),
]

_zen_common_lib = static_library(
  'zen-common',
  _zen_common_srcs,
  dependencies: _zen_common_deps,
  include_directories: _zen_common_inc,
  install: false,
)

zen_common_dep = declare_dependency(
  link_with: _zen_common_lib,
  include_directories: _zen_common_inc,
)
