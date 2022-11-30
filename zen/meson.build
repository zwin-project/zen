_zen_srcs = [
  'server.c',

  'screen/output.c',
]

_zen_deps = [
  cairo_dep,
  cglm_dep,
  m_dep,
  pixman_dep,
  toml_dep,
  wayland_server_dep,
  wlroots_dep,
  wlr_glew_renderer_dep,
  xkbcommon_dep,
  zen_common_dep,
  zgnr_dep,
]

_zen_srcs_pub = [
  protocols['xdg-shell']['private-code'],
  protocols['xdg-shell']['server-header'],
  protocols['zen-desktop']['private-code'],
  protocols['zen-desktop']['server-header'],
]

_zen_lib = static_library(
  'zen-lib',
  _zen_srcs + _zen_srcs_pub,
  install: false,
  c_args: ['-include', 'config.h'],
  dependencies: _zen_deps,
  include_directories: zen_inc,
)

zen_dep = declare_dependency(
  link_with: _zen_lib,
  dependencies: _zen_deps,
  include_directories: zen_inc,
  sources: _zen_srcs_pub,
)
