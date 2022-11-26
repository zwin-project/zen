#version 320 es
precision mediump float;

layout(binding = 0) uniform sampler2D tex;
in vec2 uv;

out vec4 outputColor;

void
main()
{
  outputColor = texture(tex, uv);
}
