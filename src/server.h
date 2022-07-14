#ifndef ZEN_SERVER_H
#define ZEN_SERVER_H

#include <wayland-server-core.h>
#include <wlr/render/wlr_renderer.h>

struct zn_server;

struct wl_event_loop *zn_server_get_loop(struct zn_server *self);

struct wlr_renderer *zn_server_get_renderer(struct zn_server *self);

/** returns exit code */
int zn_server_run(struct zn_server *self);

void zn_server_terminate(struct zn_server *self, int exit_code);

struct zn_server *zn_server_create(struct wl_display *display);

void zn_server_destroy(struct zn_server *self);

#endif  //  ZEN_SERVER_H
