subdir('shaders')

_zen_appearance_inc = include_directories('.')

_zen_appearance_srcs = [
  'base-unit.c',
  'cursor.c',
  'ray.c',
  'shader-inventory.c',
  'system.c',
  'view.c',

  'board.c',
  'board/plane-unit.c',
  
  'virtual-object.c',
  'virtual-object/gl-base-technique.c',
  'virtual-object/gl-buffer.c',
  'virtual-object/gl-program.c',
  'virtual-object/gl-sampler.c',
  'virtual-object/gl-shader.c',
  'virtual-object/gl-texture.c',
  'virtual-object/gl-vertex-array.c',
  'virtual-object/rendering-unit.c',
] + shaders

_zen_appearance_deps = [
  zen_dep,
]

_zen_appearance_lib = static_library(
  'zen-appearance',
  _zen_appearance_srcs,
  include_directories: [zen_inc, _zen_appearance_inc],
  dependencies: _zen_appearance_deps,
  install: false,
)

zen_appearance_dep = declare_dependency(
  link_with: _zen_appearance_lib,
)
