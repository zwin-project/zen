_shader_sources = [
  'board.frag',
  'board.vert',
  'bounded-nameplate.frag',
  'bounded-nameplate.vert',
  'color.frag',
  'ray.vert',
  'ray.frag',
  'view.frag',
  'view.vert',
  'view-child.frag',
  'view-child.vert',
]

shaders = []

foreach shader : _shader_sources
  var_name = shader.underscorify() + '_source'
  file_name = '@0@.h'.format(shader)

  shaders += custom_target(
    file_name,
    command: [textify, var_name, '@INPUT@', '@OUTPUT@'],
    input: shader,
    output: file_name,
  )
endforeach
