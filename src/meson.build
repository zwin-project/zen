_zen_desktop_srcs = [
  'input-device.c',
  'input-manager.c',
  'main.c',
  'output.c',
  'seat.c',
  'seat-device.c',
  'server.c',
  'view.c',
  'xdg-toplevel-view.c',
]

_zen_desktop_deps = [
  pixman_dep,
  server_protocols,
  wayland_server_dep,
  wlroots_dep,
  zen_common_dep,
  zen_scene_dep,
]

executable(
  'zen-desktop',
  _zen_desktop_srcs,
  install: true,
  dependencies: _zen_desktop_deps,
)
