_zen_desktop_srcs = [
  'main.c',
  'output.c',
  'server.c',
]

_zen_desktop_deps = [
  wayland_server_dep,
  wlroots_dep,
  zen_common_dep,
]

executable(
  'zen-desktop',
  _zen_desktop_srcs,
  install: false,
  dependencies: _zen_desktop_deps,
)
