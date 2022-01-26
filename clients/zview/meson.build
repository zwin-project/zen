srcs_zview = [
  'app.cc',
  'box.cc',
  'file-reader.cc',
  'http-reader.cc',
  'main.cc',
  'stl-object.cc',
  zigen_protocol_c,
  zigen_client_protocol_h,
  zigen_shell_protocol_c,
  zigen_shell_client_protocol_h,
  zigen_opengl_protocol_c,
  zigen_opengl_client_protocol_h,
]

deps_zview = [
  dep_libzukou_public,
  dep_curl,
]

executable(
  'zview',
  srcs_zview,
  install: true,
  dependencies: deps_zview,
)
