#ifndef ZEN_SERVER_H
#define ZEN_SERVER_H

#include <wayland-server-core.h>

struct zn_server;

/** returns exit code */
int zn_server_run(struct zn_server *self);

void zn_server_terminate(struct zn_server *self, int exit_code);

struct zn_server *zn_server_create(struct wl_display *display);

void zn_server_destroy(struct zn_server *self);

#endif  //  ZEN_SERVER_H
