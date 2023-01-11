#version 320 es
precision mediump float;

uniform mat4 zMVP;
uniform mat4 local_model;

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 uv_in;

out vec2 uv;

void
main()
{
  gl_Position = zMVP * local_model * position;
  uv = uv_in;
}
