_zen_scene_inc = include_directories('include')

_zen_scene_srcs = [
  'output.c',
  'render.c',
  'scene.c',
  'toplevel-view.c',
]

_zen_scene_deps = [
  server_protocols,
  wlroots_dep,
  wayland_server_dep,
  zen_common_dep,
]

_zen_scene_lib = static_library(
  'zen-scene',
  _zen_scene_srcs,
  dependencies: _zen_scene_deps,
  include_directories: _zen_scene_inc,
  install: false,
)

zen_scene_dep = declare_dependency(
  link_with: _zen_scene_lib,
  include_directories: _zen_scene_inc,
)
