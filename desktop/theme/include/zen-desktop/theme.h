#pragma once

#include "zen-desktop/theme/color.h"
#include "zen-desktop/theme/drop-shadow.h"
#include "zen-desktop/theme/icon.h"

struct zn_theme {
  struct {
    struct {
      struct zn_color active;
      struct zn_color inactive;
      struct zn_color active_reflection;
      struct zn_color inactive_reflection;
      struct {
        struct zn_color hover;
      } close_button;
    } header_bar;
  } color;

  struct {
    struct {
      struct zn_icon close;
    } header_bar;
  } icon;

  struct {
    struct {
      float corner;
    } header_bar;
  } radius;

  struct {
    struct {
      float height;
      float close_button;
    } header_bar;
  } size;

  struct {
    struct zn_drop_shadow view;
  } shadow;
};

struct zn_theme *zn_theme_create(void);

void zn_theme_destroy(struct zn_theme *self);
