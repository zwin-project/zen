#include "bounded/nameplate.h"
#include "zns/appearance/bounded.h"

struct zna_bounded {
  struct zns_bounded *zns_bounded;  // nonnull
  struct zna_system *system;        // nonnull

  // null when the current session does not exist, not null otherwise
  struct znr_virtual_object *virtual_object;

  struct zna_bounded_nameplate_unit *nameplate_unit;  // nonnull

  struct wl_listener session_created_listener;
  struct wl_listener session_destroyed_listener;
};
