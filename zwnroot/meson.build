_zwnr_inc = include_directories('include')

_zwnr_srcs = [
  'src/backend.c',
  'src/bounded.c',
  'src/bounded-configure.c',
  'src/compositor.c',
  'src/expansive.c',
  'src/gles-v32.c',
  'src/gl-base-technique.c',
  'src/gl-buffer.c',
  'src/gl-program.c',
  'src/gl-sampler.c',
  'src/gl-shader.c',
  'src/gl-texture.c',
  'src/gl-uniform-variable.c',
  'src/gl-vertex-array.c',
  'src/gl-vertex-attrib.c',
  'src/intersection.c',
  'src/mem-storage.c',
  'src/program-shader.c',
  'src/region.c',
  'src/region/node.c',
  'src/region/cuboid.c',
  'src/rendering-unit.c',
  'src/seat.c',
  'src/seat-ray.c',
  'src/shell.c',
  'src/shm.c',
  'src/texture-binding.c',
  'src/virtual-object.c',
]

_zwnr_srcs_pub = [
  protocols['zwin']['private-code'],
  protocols['zwin']['server-header'],
  protocols['zwin-shell']['private-code'],
  protocols['zwin-shell']['server-header'],
  protocols['zwin-gles-v32']['private-code'],
  protocols['zwin-gles-v32']['server-header'],
]

_zwnr_deps = [
  wayland_server_dep,
  zen_common_dep,
]

_zwnr_lib = static_library(
  'zwnr',
  _zwnr_srcs + _zwnr_srcs_pub,
  c_args: ['-include', 'constants.h'],
  include_directories: _zwnr_inc,
  dependencies: _zwnr_deps,
  install: false,
)

zwnr_dep = declare_dependency(
  link_with: _zwnr_lib,
  include_directories: _zwnr_inc,
  sources: _zwnr_srcs_pub,
)
