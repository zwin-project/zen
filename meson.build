project(
  'zen', 'c', 'cpp',
  version: '0.2.0-dev',
  license: 'MIT',
  meson_version: '>= 0.58.0',
  default_options: [ 'warning_level=3', 'werror=true', 'optimization=2', 'c_std=gnu11', 'cpp_std=gnu++17' ],
)

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')
cdata = configuration_data()

####################
# Global arguments #
####################

global_args = []
global_args_maybe = [
  '-D_GNU_SOURCE',
  # '-DWLR_USE_UNSTABLE',
  # '-DWL_HIDE_DEPRECATED',
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


############################################################
# Compute the relative path from build_root to source_root #
############################################################

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


################
# Dependencies #
################

#########
# Build #
#########

subdir('common')
# subdir('zen')

executable(
  'zen-desktop',
  'zen-desktop/main.cc',
  install: true,
  dependencies: [
    zen_common_dep,
    # zen_dep,
  ]
)
