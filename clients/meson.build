box_vert_glsl = custom_target(
  'box.vertex.h',
  command: [textify, 'box_vertex_shader_source', '@INPUT@', '@OUTPUT@'],
  input: 'box.vert',
  output: 'box.vert.h',
)

texture_frag_glsl = custom_target(
  'texture.fragment.h',
  command: [textify, 'texture_fragment_shader_source', '@INPUT@', '@OUTPUT@'],
  input: 'texture.frag',
  output: 'texture.fragment.h',
)

_zen_box_client_srcs = [
  'application.cc',
  'array.cc',
  'bounded.cc',
  'box.cc',
  'buffer.cc',
  'gl-base-technique.cc',
  'gl-buffer.cc',
  'gl-program.cc',
  'gl-shader.cc',
  'gl-texture.cc',
  'gl-vertex-array.cc',
  'loop.cc',
  'rendering-unit.cc',
  'shm-pool.cc',
  'virtual-object.cc',

  box_vert_glsl,
  texture_frag_glsl,

  protocols['zigen']['private-code'],
  protocols['zigen']['client-header'],
  protocols['zigen-shell']['private-code'],
  protocols['zigen-shell']['client-header'],
  protocols['zigen-gles-v32']['private-code'],
  protocols['zigen-gles-v32']['client-header'],
]

_zen_box_client_deps = [
  wayland_client_dep,
  zen_common_dep,
  glm_dep,
]

executable(
  'zen-box',
  _zen_box_client_srcs,
  cpp_args: ['-include', 'config.h'],
  install: false,
  dependencies: _zen_box_client_deps,
)
