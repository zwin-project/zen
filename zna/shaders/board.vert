#version 320 es
precision mediump float;

uniform vec2 effective_resolution;
uniform mat4 zMVP;
uniform mat4 local_model;

layout(location = 0) in vec2 pos;

out vec2 effective_position;

void
main()
{
  gl_Position = zMVP * local_model * vec4(pos, 0, 1);
  effective_position = (pos + vec2(0.5, 0)) * effective_resolution;
}
