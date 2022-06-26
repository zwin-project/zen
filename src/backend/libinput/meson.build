zen_backend_libinput_inc = include_directories('include')

zen_backend_libinput_srcs = [
  'seat.c',
]

zen_backend_libinput_deps = [
  libinput_dep,
  udev_dep,
  zen_util_dep,
]

zen_backend_libinput_lib = static_library(
  'zen-backend-libinput',
  zen_backend_libinput_srcs,
  include_directories: zen_backend_libinput_inc,
  dependencies: zen_backend_libinput_deps,
  install: false,
)

zen_backend_libinput_dep = declare_dependency(
  link_with: zen_backend_libinput_lib,
  include_directories: zen_backend_libinput_inc,
)
