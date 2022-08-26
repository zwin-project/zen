_zen_desktop_srcs = [
  'binding.c',
  'cursor.c',
  'display-system.c',
  'main.c',
  'output.c',
  'render-2d.c',
  'server.c',
  'xdg-toplevel-view.c',
  'xwayland-view.c',

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

  'wlr/box.c',

  protocols['xdg-shell']['private-code'],
  protocols['xdg-shell']['server-header'],
  protocols['zen-desktop']['private-code'],
  protocols['zen-desktop']['server-header'],
]

_zen_desktop_deps = [
  m_dep,
  pixman_dep,
  wayland_server_dep,
  wlroots_dep,
  wlr_glew_renderer_dep,
  xkbcommon_dep,
  zen_common_dep,
  zen_openvr_immersive_backend_dep,
]

executable(
  'zen-desktop',
  _zen_desktop_srcs,
  install: true,
  dependencies: _zen_desktop_deps,
  include_directories: zen_inc,
)
