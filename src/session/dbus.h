#ifndef ZEN_DBUS_H
#define ZEN_DBUS_H

#include <dbus/dbus.h>
#include <wayland-server.h>

int zn_dbus_connection_add_match_signal(DBusConnection* connection,
    const char* sender, const char* interface, const char* member,
    const char* path);

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
