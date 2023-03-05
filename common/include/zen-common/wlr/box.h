#pragma once

#include <wlr/util/box.h>

#ifdef __cplusplus
extern "C" {
#endif

void zn_wlr_fbox_closest_point(const struct wlr_fbox *box, double x, double y,
    double *dest_x, double *dest_y);

bool zn_wlr_fbox_contains_point(const struct wlr_fbox *box, double x, double y);

#ifdef __cplusplus
}
#endif
