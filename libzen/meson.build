deps_libzen = [
  dep_wayland_server,
]

srcs_libzen = [
  'callback.c',
  'compositor.c',
  'log.c',
  'virtual-object.c',
  zigen_protocol_c,
  zigen_server_protocol_h,
  zigen_shell_protocol_c,
  zigen_shell_server_protocol_h,
  zigen_opengl_protocol_c,
  zigen_opengl_server_protocol_h,
]

lib_zen = static_library(
  'zen',
  srcs_libzen,
  include_directories: public_inc,
  install: false,
  dependencies: deps_libzen,
)

dep_libzen_public = declare_dependency(
  link_with: lib_zen,
  include_directories: [ public_inc ],
)

dep_libzen_private = declare_dependency(
  link_with: lib_zen,
  include_directories: [ include_directories('.'), public_inc ],
)

subdir('opengl-renderer')
subdir('backend-glfw')
