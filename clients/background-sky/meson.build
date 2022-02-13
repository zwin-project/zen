srcs_background_sky = [
  'field.cc',
  'main.cc',
  zigen_protocol_c,
  zigen_client_protocol_h,
  zigen_shell_protocol_c,
  zigen_shell_client_protocol_h,
  zigen_opengl_protocol_c,
  zigen_opengl_client_protocol_h,
]

deps_background_sky = [
  dep_libzukou_public,
]

executable(
  'zen-background-sky',
  srcs_background_sky,
  install: true,
  dependencies: deps_background_sky,
)
