project(
  'zen', 'c',
  version: '0.0.1',
  license: 'MIT',
  meson_version: '>= 0.58.0',
  default_options: [ 'warning_level=3', 'werror=true', 'optimization=2', 'c_std=gnu11' ],
)

cc = meson.get_compiler('c')

global_args = []
global_args_maybe = [
  '-D_GNU_SOURCE',
  '-fvisibility=hidden',
]

foreach arg : global_args_maybe
  if cc.has_argument(arg)
    global_args += arg
  endif
endforeach
add_project_arguments(global_args, language: 'c')

# generic version requirements

dbus_req = '>= 1.12'
systemd_req = '>= 209'
wayland_server_req = '>= 1.18.0'

# dependencies

dbus_dep = dependency('dbus-1', version: dbus_req)
systemd_dep = dependency('libsystemd', version: systemd_req)
wayland_server_dep = dependency('wayland-server', version: wayland_server_req)

subdir('zen-util')
subdir('src')
