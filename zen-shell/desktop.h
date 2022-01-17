#ifndef ZEN_SHELL_DESKTOP_H
#define ZEN_SHELL_DESKTOP_H

#include <libzen-compositor/libzen-compositor.h>
#include <zen-shell/zen-shell.h>

struct zen_desktop_api {
  void (*move)(struct zen_cuboid_window *cuboid_window, struct zen_seat *seat,
      uint32_t serial);
  void (*rotate)(struct zen_cuboid_window *cuboid_window, versor quaternion);
};

extern struct zen_desktop_api zen_desktop_shell_interface;

#endif  //  ZEN_SHELL_DESKTOP_H
