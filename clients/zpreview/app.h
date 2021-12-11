#ifndef ZEN_CLIENT_ZPREVIEW_APP_H
#define ZEN_CLIENT_ZPREVIEW_APP_H

#include <string>

#include "viewer.h"

enum class FileFormat {
  kUnsupported = 0,
  kStl,
};

class App
{
 public:
  App(std::string filename);
  ~App();
  bool Init();
  bool Show() const;

 private:
  const std::string filename_;
  ViewerInterface *viewer_;

 private:
  FileFormat DetectFileFormat() const;
};

#endif  //  ZEN_CLIENT_ZPREVIEW_APP_H
