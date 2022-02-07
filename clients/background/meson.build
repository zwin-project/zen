srcs_background = [
  'field.cc',
  'main.cc',
  zigen_protocol_c,
  zigen_client_protocol_h,
  zigen_shell_protocol_c,
  zigen_shell_client_protocol_h,
  zigen_opengl_protocol_c,
  zigen_opengl_client_protocol_h,
]

deps_background = [
  dep_libzukou_public,
]

executable(
  'zen-background',
  srcs_background,
  install: true,
  dependencies: deps_background,
)
