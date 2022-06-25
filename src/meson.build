subdir('session')

zen_desktop_srcs = [ 'main.c' ]

zen_desktop_deps = [
  wayland_server_dep,
  zen_session_dep,
  zen_util_dep,
]

executable(
  'zen-desktop',
  zen_desktop_srcs,
  install: false,
  dependencies: zen_desktop_deps,
)
