_zgnr_inc = include_directories('include')

_zgnr_srcs = [
  'src/backend.c',
  'src/compositor.c',
  'src/virtual-object.c',

  protocols['zigen']['private-code'],
  protocols['zigen']['server-header'],
]

_zgnr_deps = [
  wayland_server_dep,
  zen_common_dep,
]

_zgnr_lib = static_library(
  'zgnr',
  _zgnr_srcs,
  include_directories: _zgnr_inc,
  dependencies: _zgnr_deps,
  install: false,
)

zgnr_dep = declare_dependency(
  link_with: _zgnr_lib,
  include_directories: _zgnr_inc,
)
