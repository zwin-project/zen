#version 320 es
precision mediump float;

uniform vec3 color;
out vec4 frag_color;

void
main()
{
  frag_color = vec4(color, 1.0);
}
