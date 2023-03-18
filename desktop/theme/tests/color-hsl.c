#include "color.c"  // NOLINT(bugprone-suspicious-include)
#include "test-harness.h"

TEST(rgb_to_hsl)
{
  vec3 rgb = GLM_VEC3_ZERO_INIT;
  vec3 hsl = GLM_VEC3_ZERO_INIT;

  rgb[0] = (float)0xa2 / 0xff;
  rgb[1] = (float)0x3a / 0xff;
  rgb[2] = (float)0xa6 / 0xff;

  rgb_to_hsl(rgb, hsl);

  int h = (int)(hsl[0] * 360);
  int s = (int)(hsl[1] * 100);
  int l = (int)(hsl[2] * 100);

  ASSERT_EQUAL_INT(297, h);
  ASSERT_EQUAL_INT(48, s);
  ASSERT_EQUAL_INT(43, l);
}

TEST(hsl_to_rgb)
{
  vec3 rgb = GLM_VEC3_ZERO_INIT;
  vec3 hsl = GLM_VEC3_ZERO_INIT;

  hsl[0] = (float)325 / 360;
  hsl[1] = (float)80 / 100;
  hsl[2] = (float)25 / 100;

  hsl_to_rgb(hsl, rgb);

  int r = (int)(rgb[0] * 0xff);
  int g = (int)(rgb[1] * 0xff);
  int b = (int)(rgb[2] * 0xff);

  ASSERT_EQUAL_INT(0x72, r);
  ASSERT_EQUAL_INT(0x0c, g);
  ASSERT_EQUAL_INT(0x48, b);
}

void
check(vec3 hsl)
{
  vec3 rgb = GLM_VEC3_ZERO_INIT;
  vec3 hsl_check = GLM_VEC3_ZERO_INIT;
  hsl_to_rgb(hsl, rgb);
  rgb_to_hsl(rgb, hsl_check);

  ASSERT_EQUAL_INT((int)(hsl[0] * 0xff), (int)(hsl_check[0] * 0xff));
  ASSERT_EQUAL_INT((int)(hsl[1] * 0xff), (int)(hsl_check[1] * 0xff));
  ASSERT_EQUAL_INT((int)(hsl[2] * 0xff), (int)(hsl_check[2] * 0xff));
}

TEST(hsl_to_hsl)
{
  check((vec3){0.03F, 0.80F, 0.24F});
  check((vec3){0.11F, 0.16F, 0.83F});

  check((vec3){0.22F, 0.20F, 0.32F});
  check((vec3){0.30F, 0.81F, 0.72F});

  check((vec3){0.39F, 0.32F, 0.44F});
  check((vec3){0.45F, 0.71F, 0.85F});

  check((vec3){0.60F, 0.19F, 0.31F});
  check((vec3){0.59F, 0.66F, 0.78F});

  check((vec3){0.70F, 0.32F, 0.24F});
  check((vec3){0.80F, 0.61F, 0.93F});

  check((vec3){0.90F, 0.18F, 0.24F});
  check((vec3){0.92F, 0.83F, 0.93F});
}
