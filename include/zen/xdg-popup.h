#pragma once

#include <wlr/types/wlr_xdg_shell.h>

#include "zen/scene/view-child.h"

struct zn_xdg_popup {
  struct zn_view_child base;
  struct wlr_xdg_popup *wlr_xdg_popup;  // nonnull

  struct wl_listener map_listener;  // TODO: popup_create時にlistenerを登録する
  struct wl_listener unmap_listener;  //
  // struct wl_listener new_popup_listener;  // TODO: popupの親の検証
  // struct wl_listener view_unmap_listener; TODO:
  struct wl_listener wlr_xdg_surface_destroy_listener;
  struct wl_listener wlr_surface_commit_listener;
};

struct zn_xdg_popup *zn_xdg_popup_create(
    struct wlr_xdg_popup *xdg_popup, struct zn_view *view);
