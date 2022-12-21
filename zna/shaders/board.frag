#version 320 es
precision mediump float;

uniform vec2 effective_resolution;

const float space = 100.0;         // px
const float dot_radius = 10.0;     // px
const float corner_radius = 50.0;  // px
const float border_width = 10.0;   // px
const vec3 color = vec3(17, 31, 77) / vec3(256, 256, 256);

const float dot_radius_2 = dot_radius * dot_radius;
const float corner_radius_2 = corner_radius * corner_radius;
const float corner_inner_radius = corner_radius - border_width;
const float corner_inner_radius_2 = corner_inner_radius * corner_inner_radius;

out vec4 frag_color;
in vec2 effective_position;

bool
within_circle(vec2 center, float radius_2, vec2 position)
{
  vec2 d = position - center;
  return d.x * d.x + d.y * d.y < radius_2;
}

bool
within_donuts(
    vec2 center, float inner_radius_2, float outer_radius_2, vec2 position)
{
  bool inner = within_circle(center, inner_radius_2, position);
  bool outer = within_circle(center, outer_radius_2, position);
  return outer && !inner;
}

void
main()
{
  float width = effective_resolution.x;
  float height = effective_resolution.y;
  float horizontal_reminder = space - mod(width, space) / 2.0;
  float vertical_reminder = space - mod(height, space) / 2.0;
  vec2 left_top_corner_center = vec2(corner_radius, corner_radius);
  vec2 left_bottom_corner_center = vec2(corner_radius, height - corner_radius);
  vec2 right_top_corner_center = vec2(width - corner_radius, corner_radius);
  vec2 right_bottom_corner_center =
      vec2(width - corner_radius, height - corner_radius);

  bool visible = false;

  // dot
  vec2 section_center;

  section_center.x = effective_position.x -
                     mod(effective_position.x + horizontal_reminder, space) +
                     space / 2.0;
  section_center.y = effective_position.y -
                     mod(effective_position.y + vertical_reminder, space) +
                     space / 2.0;

  visible = visible ||
            within_circle(section_center, dot_radius_2, effective_position);

  // boarder

  bool is_border = effective_position.x < border_width;
  is_border = is_border || effective_position.y < border_width;
  is_border = is_border || width - border_width < effective_position.x;
  is_border = is_border || height - border_width < effective_position.y;

  int corner_count = width - corner_radius < effective_position.x ? 1 : 0;
  corner_count += height - corner_radius < effective_position.y ? 1 : 0;
  corner_count += effective_position.x < corner_radius ? 1 : 0;
  corner_count += effective_position.y < corner_radius ? 1 : 0;
  bool is_corner = corner_count == 2 ? true : false;

  is_border = is_border && !is_corner;

  bool on_lt_corner_radius = within_donuts(left_top_corner_center,
      corner_inner_radius_2, corner_radius_2, effective_position);

  bool on_lb_corner_radius = within_donuts(left_bottom_corner_center,
      corner_inner_radius_2, corner_radius_2, effective_position);

  bool on_rt_corner_radius = within_donuts(right_top_corner_center,
      corner_inner_radius_2, corner_radius_2, effective_position);

  bool on_rb_corner_radius = within_donuts(right_bottom_corner_center,
      corner_inner_radius_2, corner_radius_2, effective_position);

  bool on_corner_radius =
      is_corner && (on_lt_corner_radius || on_lb_corner_radius ||
                       on_rt_corner_radius || on_rb_corner_radius);

  is_border = is_border || on_corner_radius;

  visible = visible || is_border;

  if (!visible) discard;

  frag_color = vec4(color, 1.0);
}
