_zen_srcs = [
  'src/binding.c',
  'src/buffer.c',
  'src/config.c',
  'src/cursor.c',
  'src/gl-base-technique.c',
  'src/gl-buffer.c',
  'src/gl-program.c',
  'src/gl-rendering-unit.c',
  'src/gl-shader.c',
  'src/gl-texture.c',
  'src/gl-vertex-array.c',
  'src/inode.c',
  'src/screen.c',
  'src/seat.c',
  'src/server.c',
  'src/snode.c',
  'src/snode-root.c',
  'src/view.c',
  'src/virtual-object.c',
  'src/xr-dispatcher.c',
]

zen_inc = include_directories('include')
zen_private_inc = [
  include_directories('src'),
  zen_inc,
]

_zen_pub_deps = [
  cglm_dep,
  pixman_dep,
  toml_dep,
  wayland_server_dep,
  wlroots_dep,
]

_zen_deps = [
  drm_dep,
  egl_dep,
  gl_dep,
  gbm_dep,
  glew_dep,
  m_dep,
  zen_common_dep,
  zen_wlr_glew_renderer_dep,
  xkbcommon_dep,
] + _zen_pub_deps

subdir('src/backend')

_zen_deps += zen_backend_dep

_zen_lib = static_library(
  'zen-lib',
  _zen_srcs,
  install: false,
  include_directories: zen_private_inc,
  dependencies: _zen_deps,
)

zen_dep = declare_dependency(
  link_with: _zen_lib,
  include_directories: zen_inc,
  dependencies: _zen_pub_deps,
)

if get_option('tests')
  subdir('tests')
endif
