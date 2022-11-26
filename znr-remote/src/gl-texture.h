#pragma once

#include <zen-remote/server/gl-texture.h>

#include "zen/renderer/gl-texture.h"

struct znr_gl_texture {
  std::unique_ptr<zen::remote::server::IGlTexture> proxy;

  wl_display *display;
};
