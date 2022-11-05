_zen_box_client_srcs = [
  'application.cc',
  'box.cc',
  'gl-base-technique.cc',
  'loop.cc',
  'rendering-unit.cc',
  'virtual-object.cc',

  protocols['zigen']['private-code'],
  protocols['zigen']['client-header'],
  protocols['zigen-gles-v32']['private-code'],
  protocols['zigen-gles-v32']['client-header'],
]

_zen_box_client_deps = [
  wayland_client_dep,
]

executable(
  'zen-box',
  _zen_box_client_srcs,
  install: false,
  dependencies: _zen_box_client_deps,
)
