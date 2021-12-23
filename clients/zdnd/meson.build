srcs_zdnd = [
  'box.cc',
  'main.cc',
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

deps_zdnd = [
  dep_libzukou_public,
  dep_curl,
]

executable(
  'zdnd',
  srcs_zdnd,
  install: true,
  dependencies: deps_zdnd,
)
