_zen_shell_immersive_backend_srcs = [
  'xr-shell.c',

  protocols_code['zwin-shell'],
  protocols_server_header['zwin-shell'],
]

_zen_shell_immersive_backend_deps = [
  _zen_deps,
]

_zen_shell_immersive_backend_lib = static_library(
  'zen-shell-immersive-backend',
  _zen_shell_immersive_backend_srcs,
  install: false,
  include_directories: zen_private_inc,
  dependencies: _zen_shell_immersive_backend_deps,
)

zen_shell_immersive_backend_dep = declare_dependency(
  link_with: _zen_shell_immersive_backend_lib,
)
