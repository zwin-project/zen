#ifndef ZEN_DBUS_H
#define ZEN_DBUS_H

#include <dbus/dbus.h>
#include <wayland-server.h>

/**
 * @return null on failure
 */
struct wl_event_source* zn_dbus_connection_bind(
    DBusConnection* connection, struct wl_event_loop* loop);

DBusConnection* zn_dbus_connection_create(DBusBusType type);

/**
 * @param connection
 * @param event_source nullable when not bound
 */
void zn_dbus_connection_destroy(
    DBusConnection* connection, struct wl_event_source* event_source);

#endif  //  ZEN_DBUS_H
