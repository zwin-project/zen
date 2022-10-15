project(
  'zen', 'c', 'cpp',
  version: '0.0.1-dev',
  license: 'MIT',
  meson_version: '>= 0.58.0',
  default_options: [ 'warning_level=3', 'werror=true', 'optimization=2', 'c_std=gnu11', 'cpp_std=gnu++17' ],
)

cc = meson.get_compiler('c')

global_args = []
global_args_maybe = [
  '-D_GNU_SOURCE',
  '-DWLR_USE_UNSTABLE',
  '-fvisibility=hidden',
  '-Wstrict-prototypes',
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

cglm_req = '>= 0.8.5'
openvr_req = '>= 1.12.5'
wayland_protocols_req = '>= 1.24'
wayland_server_req = '>= 1.18.0'
wlroots_req = ['>= 0.15', '< 0.16']
wlr_glew_renderer_req = wlroots_req
zen_remote_server_req = '0.1.0.6'

# dependencies

cairo_dep = dependency('cairo')
cglm_dep = dependency('cglm')
m_dep = cc.find_library('m')
pixman_dep = dependency('pixman-1')
toml_dep = dependency('libtoml')
wayland_protocols_dep = dependency('wayland-protocols', version: wayland_protocols_req)
wayland_scanner_dep = dependency('wayland-scanner')
wayland_server_dep = dependency('wayland-server', version: wayland_server_req)
wlroots_dep = dependency('wlroots', version: wlroots_req)
wlr_glew_renderer_dep = dependency('wlr-glew-renderer', version: wlr_glew_renderer_req)
xkbcommon_dep = dependency('xkbcommon')
zen_remote_server_dep = dependency('zen-remote-server', version: zen_remote_server_req)

if get_option('cui-client')
  wayland_client_dep = dependency('wayland-client')
endif

cdata = configuration_data()

if get_option('default-wallpaper')
  wallpaper_files = files(
    'assets/Zen_Wallpaper_Dark_3840x2160.png',
    'assets/Zen_Wallpaper_Gradient_3840x2160.png',
    'assets/Zen_Wallpaper_Light_3840x2160.png',
    'assets/Zen_Wallpaper_Main_3840x2160.png',
    'assets/Zen_Wallpaper_Mosaic_3840x2160.png',
  )
  wallpaper_install_dir = join_paths(datadir, 'backgrounds', 'zen')

  install_data(wallpaper_files, install_dir: wallpaper_install_dir)
  default_wallpaper = join_paths(get_option('prefix'), wallpaper_install_dir, 'Zen_Wallpaper_Main_3840x2160.png')
  cdata.set_quoted('DEFAULT_WALLPAPER', default_wallpaper)
else
  cdata.set_quoted('DEFAULT_WALLPAPER', '')
endif

zen_inc = include_directories('include')

subdir('protocols')
subdir('common')
subdir('znr')
subdir('zen')
if get_option('cui-client')
  subdir('cui-client')
endif

find_program('weston-terminal', required: true) # used in zen.desktop

install_data(
  'zen.desktop',
  install_dir: join_paths(datadir, 'wayland-sessions')
)

if get_option('tests')
  subdir('tests')
endif
