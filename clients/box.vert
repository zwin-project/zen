#version 320 es

uniform mat4 mvp;
uniform vec3 half_size;
uniform vec4 quaternion;
layout(location = 0) in vec3 position;

void
main()
{
  vec3 scaled = position * half_size;
  vec3 tmp = cross(quaternion.xyz, scaled) + quaternion.w * scaled;
  vec3 rotated = scaled + 2.0 * cross(quaternion.xyz, tmp);
  gl_Position = mvp * vec4(rotated, 1);
}
