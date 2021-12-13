#include "app.h"

#include <iostream>
#include <string>

#include "obj-viewer.h"
#include "stl-viewer.h"
#include "viewer.h"

App::App(std::string filename) : path_(filename), viewer_(nullptr) {}

App::~App()
{
  if (viewer_) delete viewer_;
}

bool
App::Init()
{
  switch (this->DetectFileFormat()) {
    case FileFormat::kStl:
      this->viewer_ = new StlViewer(path_);
      break;

    case FileFormat::kObj:
      this->viewer_ = new ObjViewer(path_);
      break;

    case FileFormat::kUnsupported:
      std::cout << this->path_ << ": file format not supported" << std::endl;
      return false;
  }

  return true;
}

bool
App::Show() const
{
  return this->viewer_->Show();
}

FileFormat
App::DetectFileFormat() const
{
  if (path_.rfind(".stl") == path_.length() - 4 ||
      path_.rfind(".STL") == path_.length() - 4)
    return FileFormat::kStl;

  if (path_.rfind(".obj") == path_.length() - 4) return FileFormat::kObj;

  return FileFormat::kUnsupported;
}
