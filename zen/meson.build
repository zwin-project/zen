_zen_srcs = [
  'binding.c',
  'config.c',
  'cursor.c',
  'decoration-manager.c',
  'main.c',
  'output.c',
  'ray.c',
  'screen-renderer.c',
  'server.c',
  'virtual-object.c',
  'xdg-decoration.c',
  'xdg-popup.c',
  'xdg-toplevel-view.c',
  'xwayland-view.c',

  'input/cursor-grab-move.c',
  'input/cursor-grab-resize.c',
  'input/input-device.c',
  'input/input-manager.c',
  'input/keyboard.c',
  'input/pointer.c',
  'input/seat.c',

  'scene/board.c',
  'scene/scene.c',
  'scene/screen.c',
  'scene/screen-layout.c',
  'scene/view.c',
  'scene/view-child.c',

  'wlr/box.c',
]

_zen_srcs_pub = [
  protocols['xdg-shell']['private-code'],
  protocols['xdg-shell']['server-header'],
  protocols['zen-desktop']['private-code'],
  protocols['zen-desktop']['server-header'],
]

_zen_deps = [
  cairo_dep,
  cglm_dep,
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
