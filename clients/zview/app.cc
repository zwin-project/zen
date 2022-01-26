#include <errno.h>
#include <string.h>

#include <iostream>
#include <string>

#include "internal.h"

App::App() : location_(""), half_size_(0.25) {}

App::~App() {}

bool
App::Show()
{
  if (!this->Connect("zigen-0")) return false;

  box_.reset(new Box(this, half_size_));

  if (location_ != "") this->UpdateLocation(location_);

  if (!this->Run()) return false;
  return true;
}

bool
App::UpdateLocation(std::string location)
{
  location_ = location;

  std::unique_ptr<StlObject> stl_object(new StlObject());
  std::unique_ptr<Reader> reader;

  if (location_.find("http") == 0) {
    std::unique_ptr<HttpReader> http_reader(new HttpReader());
    if (!http_reader->Download(location_)) return false;
    reader = move(http_reader);
  } else {
    std::unique_ptr<FileReader> file_reader(new FileReader());
    if (!file_reader->Open(location_)) return false;
    reader = move(file_reader);
  }

  stl_object->Fill(move(reader));
  box_->SetObject(move(stl_object));

  return true;
}
