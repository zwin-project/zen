#version 320 es

uniform mat4 mvp layout(location = 0) in vec4 position;

void
main()
{
  gl_Position = mvp * position;
}
