#version 320 es

uniform mat4 zMVP;
uniform mat4 local_model;
layout(location = 0) in vec2 pos;

void
main()
{
  gl_Position = zMVP * local_model * vec4(pos, 0, 1);
}
