_zgnr_inc = include_directories('include')

_zgnr_srcs = [
  'src/backend.c',
  'src/bounded.c',
  'src/bounded-configure.c',
  'src/compositor.c',
  'src/gles-v32.c',
  'src/gl-base-technique.c',
  'src/gl-buffer.c',
  'src/gl-program.c',
  'src/gl-shader.c',
  'src/gl-texture.c',
  'src/gl-uniform-variable.c',
  'src/gl-vertex-array.c',
  'src/gl-vertex-attrib.c',
  'src/mem-storage.c',
  'src/program-shader.c',
  'src/rendering-unit.c',
  'src/shell.c',
  'src/shm.c',
  'src/texture-binding.c',
  'src/virtual-object.c',
]

_zgnr_srcs_pub = [
  protocols['zigen']['private-code'],
  protocols['zigen']['server-header'],
  protocols['zigen-shell']['private-code'],
  protocols['zigen-shell']['server-header'],
  protocols['zigen-gles-v32']['private-code'],
  protocols['zigen-gles-v32']['server-header'],
]

_zgnr_deps = [
  wayland_server_dep,
  zen_common_dep,
]

_zgnr_lib = static_library(
  'zgnr',
  _zgnr_srcs + _zgnr_srcs_pub,
  c_args: ['-include', 'config.h'],
  include_directories: _zgnr_inc,
  dependencies: _zgnr_deps,
  install: false,
)

zgnr_dep = declare_dependency(
  link_with: _zgnr_lib,
  include_directories: _zgnr_inc,
  sources: _zgnr_srcs_pub,
)
