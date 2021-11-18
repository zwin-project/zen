deps_libzen = [
  dep_wayland_server,
]

srcs_libzen = [
  'compositor.c',
  'log.c',
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

subdir('backend-glfw')
