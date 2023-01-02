_zen_srcs = [
  'board.c',
  'cursor.c',
  'data-device-manager.c',
  'peer.c',
  'ray.c',
  'remote.c',
  'scene.c',
  'screen.c',
  'screen-layout.c',
  'server.c',
  'view.c',
  'virtual-object.c',

  'wlr/texture.c',

  'config/config.c',
  'config/config-parser.c',

  'screen/compositor.c',
  'screen/output.c',
  'screen/renderer.c',
  'screen/xdg-toplevel.c',
  'screen/cursor-grab/default.c',
  'screen/cursor-grab/down.c',
  'screen/cursor-grab/move.c',
  'screen/cursor-grab/resize.c',

  'input/binding.c',
  'input/input-device.c',
  'input/input-manager.c',
  'input/keyboard.c',
  'input/pointer.c',
  'input/seat.c',

  'ui/zigzag-layout.c',
  'ui/nodes/app-launcher.c',
  'ui/nodes/menu-bar.c',
  'ui/nodes/power-button.c',
  'ui/nodes/power-menu/clock.c',
  'ui/nodes/power-menu/logout.c',
  'ui/nodes/power-menu/power-menu.c',
  'ui/nodes/vr-modal/headset-dialog.c',
  'ui/nodes/vr-modal/vr-modal.c',

  'wlr/box.c',
]

_zen_srcs_pub = [
  protocols['xdg-shell']['private-code'],
  protocols['xdg-shell']['server-header'],
]

_zen_deps = [
  cairo_dep,
  cglm_dep,
  ft_dep,
  gl_dep,
  m_dep,
  pixman_dep,
  rsvg_dep,
  toml_dep,
  wayland_server_dep,
  wlroots_dep,
  wlr_glew_renderer_dep,
  xkbcommon_dep,
  zen_common_dep,
  zgnr_dep,
  zigzag_dep,
  znr_remote_dep,
]

_zen_lib = static_library(
  'zen-lib',
  _zen_srcs + _zen_srcs_pub,
  install: false,
  c_args: ['-include', 'constants.h'],
  dependencies: _zen_deps,
  include_directories: zen_inc,
)

zen_dep = declare_dependency(
  link_with: _zen_lib,
  dependencies: _zen_deps,
  include_directories: zen_inc,
  sources: _zen_srcs_pub,
)
