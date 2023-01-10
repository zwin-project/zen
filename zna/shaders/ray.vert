#version 320 es
precision mediump float;

uniform mat4 zModel;
uniform mat4 zView;
uniform mat4 zProjection;
uniform mat4 local_model;
layout(location = 0) in vec4 local_position;
layout(location = 1) in vec4 normal_in;

out vec3 normal;
out vec4 position;

void
main()
{
  vec3 origin = (zView * zModel * local_model * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
  position = zView * zModel * local_model * local_position;
  gl_Position = zProjection * position;
  normal = (zView * zModel * local_model * normal_in).xyz - origin;
}
