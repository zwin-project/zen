_zen_srcs = [
  'board.c',
  'cursor.c',
  'ray.c',
  'scene.c',
  'screen.c',
  'server.c',
  'view.c',
  'virtual-object.c',

  'screen/compositor.c',
  'screen/output.c',
  'screen/renderer.c',
  'screen/xdg-toplevel.c',
  'screen/cursor-grab/default.c',

  'input/binding.c',
  'input/input-device.c',
  'input/input-manager.c',
  'input/keyboard.c',
  'input/pointer.c',
  'input/seat.c',
]

_zen_srcs_pub = [
  protocols['xdg-shell']['private-code'],
  protocols['xdg-shell']['server-header'],
]

_zen_deps = [
  cairo_dep,
  cglm_dep,
  gl_dep,
  m_dep,
  pixman_dep,
  toml_dep,
  wayland_server_dep,
  wlroots_dep,
  wlr_glew_renderer_dep,
  xkbcommon_dep,
  zen_common_dep,
  zgnr_dep,
  znr_remote_dep,
]

_zen_lib = static_library(
  'zen-lib',
  _zen_srcs + _zen_srcs_pub,
  install: false,
  c_args: ['-include', 'config.h'],
  dependencies: _zen_deps,
  include_directories: zen_inc,
)

zen_dep = declare_dependency(
  link_with: _zen_lib,
  dependencies: _zen_deps,
  include_directories: zen_inc,
  sources: _zen_srcs_pub,
)
