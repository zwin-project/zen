srcs_obj_viewer = [
  'file-reader.cc',
  'http-reader.cc',
  'main.cc',
  'obj-parser.cc',
  'obj-viewer.cc',
  zigen_protocol_c,
  zigen_client_protocol_h,
  zigen_shell_protocol_c,
  zigen_shell_client_protocol_h,
  zigen_opengl_protocol_c,
  zigen_opengl_client_protocol_h,
]

deps_obj_viewer = [
  dep_libzukou_public,
  dep_curl,
]

executable(
  'zen-obj-viewer',
  srcs_obj_viewer,
  install: true,
  dependencies: deps_obj_viewer,
)
