srcs_background_room = [
  'file-reader.cc',
  'http-reader.cc',
  'main.cc',
  'obj-parser.cc',
  'obj-viewer.cc',
  'png-loader.cc',
  zigen_protocol_c,
  zigen_client_protocol_h,
  zigen_shell_protocol_c,
  zigen_shell_client_protocol_h,
  zigen_opengl_protocol_c,
  zigen_opengl_client_protocol_h,
]

deps_background_room = [
  dep_libzukou_public,
  dep_curl,
  dep_png,
]

executable(
  'zen-background-room',
  srcs_background_room,
  install: true,
  dependencies: deps_background_room,
)
