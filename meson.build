project(
  'zen', 'c', 'cpp',
  version: '0.0.1',
  license: 'Apache-2.0',
  meson_version: '>= 0.52.1',
  default_options:  [ 'warning_level=3', 'werror=true', 'optimization=2', 'c_std=c11' ],
)

if get_option('debug')
  add_global_arguments('-DDEBUG', language : [ 'c', 'cpp' ])
endif

public_inc = include_directories('include')

# dependencies
dep_wayland_server = dependency('wayland-server')
dep_wayland_client = dependency('wayland-client')
dep_wayland_scanner = dependency('wayland-scanner', version: '>= 1.14.91')
dep_glfw3 = dependency('glfw3')
dep_glew = dependency('glew')
dep_zgn_protocol = dependency('zigen-protocols')
dep_cglm = dependency('cglm')

subdir('protocol')
subdir('libzen')
subdir('zen')
subdir('tests')

if get_option('clients')
  subdir('clients')
endif
