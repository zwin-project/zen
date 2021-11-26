deps_zen_shell = [
  dep_libzen_compositor_public,
]

srcs_zen_shell = [
  'cuboid-window.c',
  'shell.c',
  zigen_shell_protocol_c,
  zigen_shell_server_protocol_h,
]

lib_zen_shell = static_library(
  'zigen-shell',
  srcs_zen_shell,
  install: false,
  dependencies: deps_zen_shell,
)

dep_zen_shell_public = declare_dependency(
  link_with: lib_zen_shell,
  include_directories: public_inc,
)
