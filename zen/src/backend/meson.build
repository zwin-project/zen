subdir('immersive/gl')
subdir('immersive/remote')

_zen_backend_srcs = [
  'default-backend.c',

  'immersive/client-virtual-object.c',
  'immersive/shm.c',
  'immersive/shm-buffer.c',
  'immersive/shm-pool.c',
  'immersive/xr.c',
  'immersive/xr-compositor.c',

  'screen/compositor.c',
  'screen/keyboard.c',
  'screen/layer-shell.c',
  'screen/layer-surface.c',
  'screen/output.c',
  'screen/pointer.c',
  'screen/surface-snode.c',
  'screen/xwayland-surface.c',

  protocols_code['zwin'],
  protocols_server_header['zwin'],
]

_zen_backend_deps = [
  _zen_deps,
  zen_gl_immersive_backend_dep,
  zen_remote_immersive_backend_dep,
]

_zen_backend_lib = static_library(
  'zen-backend',
  _zen_backend_srcs,
  install: false,
  include_directories: zen_private_inc,
  dependencies: _zen_backend_deps,
)

zen_backend_dep = declare_dependency(
  link_with: _zen_backend_lib,
)
