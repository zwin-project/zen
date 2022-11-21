#version 320 es

uniform mat4 mvp;
uniform vec4 translate;
layout(location = 0) in vec4 position;

void
main()
{
  gl_Position = mvp * (translate + position);
}
