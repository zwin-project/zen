project(
  'zen', 'c', 'cpp',
  version: '0.1.3-dev',
  license: 'MIT',
  meson_version: '>= 0.58.0',
  default_options: [ 'warning_level=3', 'werror=true', 'optimization=2', 'c_std=gnu11', 'cpp_std=gnu++17' ],
)

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')
cdata = configuration_data()

global_args = []
global_args_maybe = [
  '-D_GNU_SOURCE',
  '-DWLR_USE_UNSTABLE',
  '-DWL_HIDE_DEPRECATED',
  '-fvisibility=hidden',
  '-Wstrict-prototypes',
  '-Wno-gnu-zero-variadic-macro-arguments',
]

foreach arg : global_args_maybe
  if cc.has_argument(arg)
    global_args += arg
  endif
endforeach
add_project_arguments(global_args, language: 'c')

global_args = []
foreach arg : global_args_maybe
  if cxx.has_argument(arg)
    global_args += arg
  endif
endforeach
add_project_arguments(global_args, language: 'cpp')

have_funcs = [
  'memfd_create',
  'posix_fallocate',
]

foreach func : have_funcs
  cdata.set('HAVE_' + func.underscorify().to_upper(), cc.has_function(func))
endforeach

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
wayland_req = '>= 1.18.0'
wlroots_req = ['>= 0.15', '< 0.16']
zen_remote_server_req = '>= 0.1.1'
zwin_protocols_req = '0.1.0'

glew_proj = subproject(
  'glew',
  required: true,
  version: '2.2.0',
)

# dependencies

cairo_dep = dependency('cairo')
cglm_dep = dependency('cglm')
drm_dep = dependency('libdrm')
gbm_dep = dependency('gbm')
egl_dep = dependency('egl')
fontconfig_dep = dependency('fontconfig')
glew_dep = glew_proj.get_variable('glew_dep')
gl_dep = dependency('gl')
m_dep = cc.find_library('m')
pixman_dep = dependency('pixman-1')
wayland_protocols_dep = dependency('wayland-protocols', version: wayland_protocols_req)
wayland_scanner_dep = dependency('wayland-scanner')
wayland_server_dep = dependency('wayland-server', version: wayland_req)
wlroots_dep = dependency('wlroots', version: wlroots_req)
xkbcommon_dep = dependency('xkbcommon')
zen_remote_server_dep = dependency('zen-remote-server', version: zen_remote_server_req)
zwin_protocols_dep = dependency('zwin-protocols', version: zwin_protocols_req)
ft_dep = dependency('freetype2')
rsvg_dep = dependency('librsvg-2.0')

if get_option('cui-client')
  wayland_client_dep = dependency('wayland-client', version: wayland_req)
endif

icons = {
  'close.svg'         : 'CLOSE_ICON',
  'power-button.svg'  : 'POWER_BUTTON_ICON',
  'unknown-app.png'   : 'UNKNOWN_APP_ICON',
  'vr-blue.svg'       : 'VR_ICON_BLUE',
  'vr-translucent.svg': 'VR_ICON_TRANSLUCENT',
  'vr-white.svg'      : 'VR_ICON_WHITE',
}

icon_install_dir = join_paths(datadir, 'zen')
foreach filename, definition : icons
  file = join_paths('assets', 'icon', filename)

  install_data(file, install_dir: icon_install_dir)

  installed_path = join_paths(get_option('prefix'), icon_install_dir, filename)

  cdata.set_quoted(definition, installed_path)
endforeach

if get_option('default-wallpaper')
  wallpaper_files = files(
    'assets/wallpaper/Zen_Wallpaper_Dark_3840x2160.png',
    'assets/wallpaper/Zen_Wallpaper_Gradient_3840x2160.png',
    'assets/wallpaper/Zen_Wallpaper_Light_3840x2160.png',
    'assets/wallpaper/Zen_Wallpaper_Main_3840x2160.png',
    'assets/wallpaper/Zen_Wallpaper_Mosaic_3840x2160.png',
  )
  wallpaper_install_dir = join_paths(datadir, 'backgrounds', 'zen')

  install_data(wallpaper_files, install_dir: wallpaper_install_dir)
  default_wallpaper = join_paths(get_option('prefix'), wallpaper_install_dir, 'Zen_Wallpaper_Main_3840x2160.png')
  cdata.set_quoted('DEFAULT_WALLPAPER', default_wallpaper)
else
  cdata.set_quoted('DEFAULT_WALLPAPER', '')
endif

configure_file(
  output: 'constants.h',
  configuration: cdata,
)

textify = files('tools/textify.sh')[0]

zen_inc = include_directories('include')

subdir('3rd-party')
subdir('protocols')
subdir('common')
subdir('zwnroot')
subdir('zigzag')
subdir('znr-remote')
subdir('zen')
subdir('zns')
subdir('zna')

install_data(
  'zen.desktop',
  install_dir: join_paths(datadir, 'wayland-sessions')
)

if get_option('tests')
  subdir('tests')
endif

executable(
  'zen-desktop',
  'zen/main.c',
  install: true,
  dependencies: [
    zen_dep,
    zen_appearance_dep,
    zen_shell_dep,
  ],
)
