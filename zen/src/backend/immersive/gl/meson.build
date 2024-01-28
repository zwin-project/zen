_zen_gl_immersive_backend_srcs = [
  'client-gl-base-technique.c',
  'client-gl-buffer.c',
  'client-gl-program.c',
  'client-gl-rendering-unit.c',
  'client-gl-shader.c',
  'client-gl-texture.c',
  'client-gl-vertex-array.c',
  'gl-context.c',

  protocols_code['zwin-gl'],
  protocols_server_header['zwin-gl'],
]

_zen_gl_immersive_backend_deps = [
  _zen_deps,
]

_zen_gl_immersive_backend_lib = static_library(
  'zen-gl-immersive-backend',
  _zen_gl_immersive_backend_srcs,
  install: false,
  include_directories: zen_private_inc,
  dependencies: _zen_gl_immersive_backend_deps,
)

zen_gl_immersive_backend_dep = declare_dependency(
  link_with: _zen_gl_immersive_backend_lib,
)
