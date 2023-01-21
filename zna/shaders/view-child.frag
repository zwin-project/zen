#version 320 es
precision mediump float;

uniform sampler2D view_image;

in vec2 uv;
out vec4 frag_color;

void
main()
{
  vec4 color = texture(view_image, uv);
  if (color.a < 0.5) {
    discard;
  }
  frag_color = color;
}
