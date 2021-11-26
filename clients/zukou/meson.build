deps_zukou = [
  dep_wayland_client,
]

srcs_zukou = [
  'app.cc',
  'cuboid-window.cc',
  'glm-helper.cc',
  'virtual-object.cc',
  'opengl-component.cc',
  zigen_protocol_c,
  zigen_client_protocol_h,
  zigen_shell_protocol_c,
  zigen_shell_client_protocol_h,
  zigen_opengl_protocol_c,
  zigen_opengl_client_protocol_h,
]

lib_zukou = static_library(
  'zukou',
  srcs_zukou,
  include_directories: client_inc,
  install: false,
  dependencies: deps_zukou,
)

dep_libzukou_public = declare_dependency(
  link_with: lib_zukou,
  include_directories: client_inc,
)
