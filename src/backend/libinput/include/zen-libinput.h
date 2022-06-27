#ifndef ZEN_BACKEND_LIBINPUT_H
#define ZEN_BACKEND_LIBINPUT_H

#include <libudev.h>
#include <zen-session.h>

struct zn_libinput;

struct zn_libinput *zn_libinput_create(
    struct udev *udev, struct zn_session *session, const char *seat_id);

void zn_libinput_destroy(struct zn_libinput *self);

#endif  //  ZEN_BACKEND_LIBINPUT_H
