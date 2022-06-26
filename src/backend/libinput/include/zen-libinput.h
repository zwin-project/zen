#ifndef ZEN_BACKEND_LIBINPUT_H
#define ZEN_BACKEND_LIBINPUT_H

#include <libudev.h>

struct zn_libinput;

struct zn_libinput *zn_libinput_create(struct udev *udev);

void zn_libinput_destroy(struct zn_libinput *self);

#endif  //  ZEN_BACKEND_LIBINPUT_H
