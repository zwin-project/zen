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

# Compute the relative path from build_root to source_root

source_root = meson.current_source_dir().split('/')
build_root = meson.global_build_root().split('/')

slen = source_root.length()
blen = build_root.length()

relative_source_parts = []

i = 0
in_prefix = true
foreach p : build_root
  if not in_prefix or i >= slen or p != source_root[i]
    in_prefix = false
    relative_source_parts += '..'
  endif
  i += 1
endforeach

i = 0
in_prefix = true
foreach p : source_root
  if not in_prefix or i >= blen or build_root[i] != p
    in_prefix = false
    relative_source_parts += p
  endif
  i += 1
endforeach

relative_source_dir = join_paths(relative_source_parts) + '/'

# If relative_source_dir is used as the prefix of preprocessor macros such as __FILE__,
# Replace that with an empty string.
if cc.has_argument('-fmacro-prefix-map=/prefix/to/hide=')
	add_project_arguments(
		'-fmacro-prefix-map=@0@='.format(relative_source_dir),
		language: 'c',
	)
endif

datadir = get_option('datadir')

# generic version requirements

wayland_server_req = '>= 1.18.0'
wayland_protocols_req = '>= 1.24'
wlroots_req = ['>= 0.15', '< 0.16']

# dependencies

wayland_server_dep = dependency('wayland-server', version: wayland_server_req)
wayland_scanner_dep = dependency('wayland-scanner')
wayland_protocols_dep = dependency('wayland-protocols', version: wayland_protocols_req)
wlroots_dep = dependency('wlroots', version: wlroots_req)
pixman_dep = dependency('pixman-1')

zen_inc = include_directories('include')

subdir('protocols')
subdir('common')
subdir('zen')

find_program('weston-terminal', required: true) # used in zen.desktop

install_data(
  'zen.desktop',
  install_dir: join_paths(datadir, 'wayland-sessions')
)
