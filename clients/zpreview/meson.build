srcs_zpreview = [
  'app.cc',
  'main.cc',
  'obj-object.cc',
  'obj-viewer.cc',
  'stl-object.cc',
  'stl-viewer.cc',
  'file-reader.cc',
  'http-reader.cc',
  zigen_protocol_c,
  zigen_client_protocol_h,
  zigen_shell_protocol_c,
  zigen_shell_client_protocol_h,
  zigen_opengl_protocol_c,
  zigen_opengl_client_protocol_h,
]

deps_zpreview = [
  dep_libzukou_public,
  dep_curl,
]

executable(
  'zpreview',
  srcs_zpreview,
  install: true,
  dependencies: deps_zpreview,
)
