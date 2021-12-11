srcs_simple_box = [
  'box.cc',
  'main.cc',
  zigen_protocol_c,
  zigen_client_protocol_h,
  zigen_shell_protocol_c,
  zigen_shell_client_protocol_h,
  zigen_opengl_protocol_c,
  zigen_opengl_client_protocol_h,
]

deps_simple_box = [
  dep_libzukou_public,
]

executable(
  'zen-simple-box',
  srcs_simple_box,
  install: true,
  dependencies: deps_simple_box,
)
