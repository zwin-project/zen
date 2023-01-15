# Build as static library with egl, no installation

project(
  'glew',
  'c',
  version : '2.2.0',
  license: ['MIT', 'BSD-3'],
  meson_version : '>= 0.43.0'
)

cc = meson.get_compiler('c')
host_system = host_machine.system()

user_args = [
  '-DGLEW_STATIC',
]

c_args = [
  '-DGLEW_NO_GLU',
  '-DGLEW_STATIC',
  '-DGLEW_EGL',
]

c_args += cc.get_supported_arguments([
  '-ansi',
  '-W',
  '-Wcast-qual',
  '-fno-stack-protector',
  '-Wno-visibility',
])

glew_sources = [
  'include/GL/wglew.h',
  'include/GL/glew.h',
  'include/GL/glxew.h',
  'src/glew.c',
]

dep_gl = dependency('gl')
dep_egl = dependency('egl')

glew_include = include_directories('include')

libglew = static_library(
  'GLEW',
  glew_sources,
  c_args : c_args,
  include_directories : glew_include,
  dependencies : [dep_gl, dep_egl],
  install : false,
)

glew_dep = declare_dependency(
  link_with : libglew,
  compile_args: user_args,
  include_directories : glew_include,
  dependencies : [dep_gl, dep_egl],
)
