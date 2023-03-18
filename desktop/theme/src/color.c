#include "zen-desktop/theme/color.h"

#include <cglm/vec3.h>

#include "zen-common/log.h"
#include "zen-common/util.h"

static void
rgb_to_hsl(vec3 rgb, vec3 hsl)
{
  float unused = 0;
  float max = glm_vec3_max(rgb);
  float min = glm_vec3_min(rgb);
  float delta = max - min;

  if (delta == 0) {
    hsl[0] = 0;
  } else if (max == rgb[0]) {
    hsl[0] = (rgb[1] - rgb[2]) / delta;
  } else if (max == rgb[1]) {
    hsl[0] = 2 + (rgb[2] - rgb[0]) / delta;
  } else if (max == rgb[2]) {
    hsl[0] = 4 + (rgb[0] - rgb[1]) / delta;
  }

  hsl[0] = modff(hsl[0] / 6.F + 1.F, &unused);

  float cnt = (max + min) / 2;
  if (cnt < 0.5) {
    hsl[1] = (cnt - min) / cnt;
  } else {
    hsl[1] = (max - cnt) / (1 - cnt);
  }

  hsl[2] = (max + min) / 2;
}

static void
hsl_to_rgb(const vec3 hsl, vec3 rgb)
{
  float max = 0;
  float min = 0;

  if (hsl[2] < 0.5) {
    max = hsl[2] + hsl[2] * hsl[1];
    min = hsl[2] - hsl[2] * hsl[1];
  } else {
    max = hsl[2] + (1 - hsl[2]) * hsl[1];
    min = hsl[2] - (1 - hsl[2]) * hsl[1];
  }

  if (hsl[0] * 6 < 1) {
    rgb[0] = max;
    rgb[1] = (hsl[0] * 6) * (max - min) + min;
    rgb[2] = min;
  } else if (hsl[0] * 6 < 2) {
    rgb[0] = ((2 - hsl[0] * 6)) * (max - min) + min;
    rgb[1] = max;
    rgb[2] = min;
  } else if (hsl[0] * 6 < 3) {
    rgb[0] = min;
    rgb[1] = max;
    rgb[2] = (hsl[0] * 6 - 2) * (max - min) + min;
  } else if (hsl[0] * 6 < 4) {
    rgb[0] = min;
    rgb[1] = ((4 - hsl[0] * 6)) * (max - min) + min;
    rgb[2] = max;
  } else if (hsl[0] * 6 < 5) {
    rgb[0] = ((hsl[0] * 6 - 4)) * (max - min) + min;
    rgb[1] = min;
    rgb[2] = max;
  } else if (hsl[0] * 6 < 6) {
    rgb[0] = max;
    rgb[1] = min;
    rgb[2] = (6 - hsl[0] * 6) * (max - min) + min;
  }
}

float
zn_color_get_lightness(struct zn_color *self)
{
  vec3 hsl;
  rgb_to_hsl(self->rgba, hsl);
  return hsl[2];
}

void
zn_color_set_lightness(struct zn_color *self, float lightness)
{
  vec3 hsl;
  rgb_to_hsl(self->rgba, hsl);
  hsl[2] = lightness;
  hsl_to_rgb(hsl, self->rgba);
}

void
zn_color_set_cairo_source(struct zn_color *self, cairo_t *cr)
{
  cairo_set_source_rgba(
      cr, self->rgba[0], self->rgba[1], self->rgba[2], self->rgba[3]);
}

void
zn_color_init(struct zn_color *self, uint32_t rgba)
{
  self->rgba[0] = (float)(rgba >> 24) / 0xff;
  self->rgba[1] = (float)((rgba >> 16) & 0xff) / 0xff;
  self->rgba[2] = (float)((rgba >> 8) & 0xff) / 0xff;
  self->rgba[3] = (float)(rgba & 0xff) / 0xff;
}

void
zn_color_fini(struct zn_color *self UNUSED)
{}
