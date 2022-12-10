#version 320 es

uniform mat4 zMVP;
layout(location = 0) in float i;
uniform vec3 origin;
uniform vec3 tip;

void
main()
{
  gl_Position = zMVP * vec4(origin * (1.0 - i) + tip * i, 1);
}
