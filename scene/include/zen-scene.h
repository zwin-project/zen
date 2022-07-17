#ifndef ZEN_SCENE_H
#define ZEN_SCENE_H

#include <wlr/types/wlr_xdg_shell.h>

struct zn_scene;
struct zn_scene_output;
struct zn_scene_toplevel_view;

/* zn_scene */

struct zn_scene* zn_scene_create();

void zn_scene_destroy(struct zn_scene* self);

/* zn_scene_output */

struct zn_scene_output* zn_scene_output_create(struct zn_scene* scene);

void zn_scene_output_destroy(struct zn_scene_output* self);

/* zn_scene_toplevel_view */

/**
 * caller of this function must call zn_scene_toplevel_view_destroy when the
 * given wlr_xdg_toplevel is destroyed
 */
struct zn_scene_toplevel_view* zn_scene_toplevel_view_create(
    struct zn_scene* scene, struct wlr_xdg_toplevel* wlr_xdg_toplevel,
    struct zn_scene_output* output /* nullable */);

void zn_scene_toplevel_view_destroy(struct zn_scene_toplevel_view* self);

#endif  //  ZEN_SCENE_H
