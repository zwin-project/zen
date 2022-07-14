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
  '-DWLR_USE_UNSTABLE',
  '-fvisibility=hidden',
]

foreach arg : global_args_maybe
  if cc.has_argument(arg)
    global_args += arg
  endif
endforeach
add_project_arguments(global_args, language: 'c')

# generic version requirements

wayland_server_req = '>= 1.18.0'
wlroots_req = ['>= 0.15', '< 0.16']

# dependencies

wayland_server_dep = dependency('wayland-server', version: wayland_server_req)
wlroots_dep = dependency('wlroots', version: wlroots_req)
pixman_dep = dependency('pixman-1')

subdir('common')
subdir('src')
