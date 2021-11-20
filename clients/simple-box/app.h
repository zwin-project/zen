#ifndef ZGN_CLIENT_APP_H
#define ZGN_CLIENT_APP_H

#include "zgn-window.h"

class App
{
 public:
  App();
  ~App();
  bool Start();
  ZgnWindow<App> *zgn_window();

 private:
  ZgnWindow<App> *zgn_window_;
};

inline ZgnWindow<App> *
App::zgn_window()
{
  return zgn_window_;
}

#endif  //  ZGN_CLIENT_APP_H
