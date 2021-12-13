#include "stl-viewer.h"

#include <iostream>
#include <string>
#include <vector>

#include "file-reader.h"
#include "http-reader.h"
#include "reader.h"
#include "stl-object.h"

StlViewer::StlViewer(std::string path) : ViewerInterface(), object_(nullptr)
{
  path_ = path;
  app_ = new zukou::App();
}

StlViewer::~StlViewer()
{
  if (object_) delete object_;
  delete app_;
}

bool
StlViewer::Show()
{
  std::vector<StlTriangle> triangles;
  uint32_t triangle_count;
  StlTriangle buffer;
  Reader *reader;
  char unused[80];

  if (path_.find("http") == 0) {
    HttpReader *http_reader = new HttpReader();
    if (!http_reader->Download(path_)) return false;
    reader = http_reader;
  } else {
    FileReader *file_reader = new FileReader();
    if (!file_reader->Open(path_)) return false;
    reader = file_reader;
  }

  if (!reader->Read(unused, 80)) goto broken_file;

  if (std::string(unused, 5) == "solid") {
    std::cerr << path_
              << ": this file seems \"ASCII\" STL which is not "
                 "supported yet"
              << std::endl;
    goto err;
  }

  if (!reader->Read(
          reinterpret_cast<char *>(&triangle_count), sizeof(triangle_count)))
    goto broken_file;

  if (triangle_count <= 0) goto broken_file;

  triangles.reserve(triangle_count);

  for (uint32_t i = 0; i < triangle_count; i++) {
    if (!reader->Read(reinterpret_cast<char *>(&buffer), sizeof(StlTriangle)))
      goto broken_file;
    triangles.push_back(buffer);
  }

  delete reader;

  if (app_->Connect("zigen-0") == false) return false;

  object_ = new StlObject(app_, triangles, glm::vec3(0.25f));

  std::vector<StlTriangle>().swap(triangles);  // free triangles

  return app_->Run();

broken_file:
  std::cerr << "broken file: " << path_ << std::endl;

err:
  delete reader;
  return false;
}
