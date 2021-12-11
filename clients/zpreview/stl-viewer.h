#ifndef ZEN_CLIENT_ZPREVIEW_STL_VIEWER_H
#define ZEN_CLIENT_ZPREVIEW_STL_VIEWER_H

#include <zukou.h>

#include <string>

#include "stl-object.h"
#include "viewer.h"

class StlViewer : public ViewerInterface
{
 public:
  StlViewer(std::string filename);
  ~StlViewer();
  bool Show();

 private:
  std::string filename_;
  zukou::App *app_;
  StlObject *object_;
};

#endif  //  ZEN_CLIENT_ZPREVIEW_STL_VIEWER_H
