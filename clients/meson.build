_zen_box_client_srcs = [
  'application.cc',
  'box.cc',
  'buffer.cc',
  'gl-base-technique.cc',
  'gl-buffer.cc',
  'gl-vertex-array.cc',
  'loop.cc',
  'rendering-unit.cc',
  'shm-pool.cc',
  'virtual-object.cc',

  protocols['zigen']['private-code'],
  protocols['zigen']['client-header'],
  protocols['zigen-gles-v32']['private-code'],
  protocols['zigen-gles-v32']['client-header'],
]

_zen_box_client_deps = [
  wayland_client_dep,
  zen_common_dep,
]

executable(
  'zen-box',
  _zen_box_client_srcs,
  cpp_args: ['-include', 'config.h'],
  install: false,
  dependencies: _zen_box_client_deps,
)
