#version 320 es
precision mediump float;

uniform vec4 color;

out vec4 outputColor;

void
main()
{
  outputColor = color;
}
