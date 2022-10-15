build_config_h = configure_file(
  input: 'build-config.h.meson',
  output: 'build-config.h',
  configuration: cdata
)

_zen_desktop_srcs = [
  build_config_h,

  'binding.c',
  'config.c',
  'cursor.c',
  'decoration-manager.c',
  'main.c',
  'output.c',
  'screen-renderer.c',
  'server.c',
  'xdg-decoration.c',
  'xdg-popup.c',
  'xdg-toplevel-view.c',
  'xwayland-view.c',

  'immersive/remote/display-system.c',
  'immersive/remote/object/board.c',
  'immersive/remote/object/ray.c',
  'immersive/remote/renderer.c',
  'immersive/remote/scene.c',

  'input/cursor-grab-move.c',
  'input/cursor-grab-resize.c',
  'input/input-device.c',
  'input/input-manager.c',
  'input/keyboard.c',
  'input/pointer.c',
  'input/seat.c',

  'scene/board.c',
  'scene/ray.c',
  'scene/scene.c',
  'scene/screen.c',
  'scene/screen-layout.c',
  'scene/view.c',
  'scene/view-child.c',

  'wlr/box.c',

  protocols['xdg-shell']['private-code'],
  protocols['xdg-shell']['server-header'],
  protocols['zen-desktop']['private-code'],
  protocols['zen-desktop']['server-header'],
]

_zen_desktop_deps = [
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
  znr_dep,
]

executable(
  'zen-desktop',
  _zen_desktop_srcs,
  install: true,
  dependencies: _zen_desktop_deps,
  include_directories: zen_inc,
)
