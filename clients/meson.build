default_vert_glsl = custom_target(
  'default.vertex.h',
  command: [textify, 'default_vertex_shader_source', '@INPUT@', '@OUTPUT@'],
  input: 'default.vert',
  output: 'default.vert.h',
)

color_frag_glsl = custom_target(
  'color.fragment.h',
  command: [textify, 'color_fragment_shader_source', '@INPUT@', '@OUTPUT@'],
  input: 'color.frag',
  output: 'color.fragment.h',
)

_zen_box_client_srcs = [
  'application.cc',
  'bounded.cc',
  'box.cc',
  'buffer.cc',
  'gl-base-technique.cc',
  'gl-buffer.cc',
  'gl-program.cc',
  'gl-shader.cc',
  'gl-vertex-array.cc',
  'loop.cc',
  'rendering-unit.cc',
  'shm-pool.cc',
  'virtual-object.cc',

  default_vert_glsl,
  color_frag_glsl,

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
