#include "app.h"

#include <stdio.h>
#include <string.h>
#include <wayland-client.h>
#include <zigen-client-protocol.h>

#include "box.h"
#include "zgn-virtual-object.h"
#include "zgn-window.h"

App::App() { zgn_window_ = new ZgnWindow<App>(this); }

App::~App() { delete zgn_window_; }

bool
App::Start()
{
  bool ret;
  if (zgn_window()->Connect("zigen-0") == false) {
    fprintf(stderr, "failed to connect to zigen compositor\n");
    return false;
  }

  Box *box = new Box(this);
  box->virtual_object()->NextFrame();

  ret = zgn_window()->Run();

  delete box;

  return ret;
}
