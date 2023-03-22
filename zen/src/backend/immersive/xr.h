#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct zn_xr {
  void *impl_data;  // used by the impl object

  struct {
    struct wl_signal new_system;  // (struct zn_xr_system *)
  } events;
};

struct zn_xr *zn_xr_create(struct wl_display *display);

void zn_xr_destroy(struct zn_xr *self);

#ifdef __cplusplus
}
#endif
