_zen_box_client_srcs = [
  'application.cc',
  'box.cc',
  'loop.cc',
  'virtual-object.cc',

  protocols['zigen']['private-code'],
  protocols['zigen']['client-header'],
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
