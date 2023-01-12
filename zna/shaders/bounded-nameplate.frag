#version 320 es
precision mediump float;

uniform sampler2D image;

in vec2 uv;

out vec4 frag_color;

void
main()
{
  frag_color = texture(image, uv);
  if (frag_color.a < 0.5) discard;
}
