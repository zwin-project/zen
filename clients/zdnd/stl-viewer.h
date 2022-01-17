#ifndef ZEN_CLIENT_ZDND_STL_VIEWER_H
#define ZEN_CLIENT_ZDND_STL_VIEWER_H

#include <zukou.h>

#include <string>

#include "stl-object.h"
#include "viewer.h"

class StlViewer : public ViewerInterface
{
 public:
  StlViewer(std::string path);
  ~StlViewer();
  bool Show();

 private:
  std::string path_;
  zukou::App *app_;
  StlObject *object_;
};

#endif  //  ZEN_CLIENT_ZDND_STL_VIEWER_H
