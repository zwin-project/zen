#include <wayland-server-core.h>
#include <wlr/types/wlr_output.h>

#include "zen/screen.h"

struct zn_output {
  struct zn_screen *screen;  // @nonnull, @owning

  struct wlr_output *wlr_output;  // @nonnull, @outlive

  struct wl_listener wlr_output_destroy_listener;
};

struct zn_output *zn_output_create(struct wlr_output *wlr_output);
