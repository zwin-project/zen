#version 320 es
precision mediump float;

uniform mat4 zView;

const vec3 global_light = vec3(0, 10, 0);
const vec3 Ka = vec3(0.8, 0.8, 0.8);
const vec3 Kd = vec3(0.8, 0.8, 0.8);
const vec3 Ks = vec3(0.5, 0.5, 0.5);
const float alpha = 32.0;

in vec3 normal;
in vec4 position;

out vec4 frag_color;

void
main()
{
  // view coordinates;
  vec3 view = -normalize(position.xyz);
  vec3 light = normalize((zView * vec4(global_light, 1.0) - position).xyz);
  vec3 half_way = normalize(light + view);
  vec3 norm = normalize(normal);

  vec3 ambient = Ka;
  vec3 diffuse = Kd * max(dot(light, norm), 0.0);
  vec3 specular = Ks * pow(max(dot(half_way, norm), 0.0), alpha);

  frag_color = vec4(ambient + diffuse + specular, 1.0);
}
