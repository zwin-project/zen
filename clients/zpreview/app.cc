#include "app.h"

#include <iostream>
#include <string>

#include "stl-viewer.h"
#include "viewer.h"

App::App(std::string filename) : filename_(filename), viewer_(nullptr) {}

App::~App()
{
  if (viewer_) delete viewer_;
}

bool
App::Init()
{
  switch (this->DetectFileFormat()) {
    case FileFormat::kStl:
      this->viewer_ = new StlViewer(filename_);
      break;

    case FileFormat::kUnsupported:
      std::cout << this->filename_ << ": file format not supported"
                << std::endl;
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
  if (filename_.rfind(".stl") == filename_.length() - 4)
    return FileFormat::kStl;
  return FileFormat::kUnsupported;
}
