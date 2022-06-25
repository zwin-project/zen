zen_session_inc = include_directories('include')

zen_session_srcs = [
  'dbus.c',
  'session-logind.c'
]

zen_session_deps = [
  dbus_dep,
  systemd_dep,
  wayland_server_dep,
  zen_util_dep,
]

zen_session_lib = static_library(
  'zen-session',
  zen_session_srcs,
  include_directories: zen_session_inc,
  dependencies: zen_session_deps,
  install: false,
)

zen_session_dep = declare_dependency(
  link_with: zen_session_lib,
  include_directories: zen_session_inc,
)
