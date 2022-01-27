deps_zen_compositor = [
  dep_cglm,
  dep_wayland_server,
  dep_libinput,
  dep_udev,
  deb_xkbcommon,
]

srcs_zen_compositor = [
  'callback.c',
  'cglm-helper.c',
  'compositor.c',
  'config.c',
  'file.c',
  'ray-client.c',
  'ray.c',
  'keyboard-client.c',
  'keyboard.c',
  'log.c',
  'seat.c',
  'udev-seat.c',
  'virtual-object.c',
  'weak-link.c',
  zigen_protocol_c,
  zigen_server_protocol_h,
]

lib_zen_compositor = static_library(
  'zen-compositor',
  srcs_zen_compositor,
  include_directories: public_inc,
  install: false,
  dependencies: deps_zen_compositor,
)

dep_libzen_compositor_public = declare_dependency(
  link_with: lib_zen_compositor,
  include_directories: [ public_inc ],
  dependencies: deps_zen_compositor,
)

dep_libzen_compositor_private = declare_dependency(
  link_with: lib_zen_compositor,
  include_directories: [ include_directories('.'), public_inc ],
  dependencies: deps_zen_compositor,
)
