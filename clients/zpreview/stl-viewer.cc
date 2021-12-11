#include "stl-viewer.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "stl-object.h"

StlViewer::StlViewer(std::string filename) : ViewerInterface(), object_(nullptr)
{
  filename_ = filename;
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
  char unused[80];
  uint32_t triangle_count;
  std::ifstream ifs(filename_, std::ios::in | std::ios::binary);
  std::vector<StlTriangle> triangles;
  StlTriangle buffer;

  if (ifs.fail()) {
    std::cerr << "failed to open file: " << filename_ << std::endl;
    return false;
  }

  ifs.read(unused, 80);
  ifs.read(reinterpret_cast<char *>(&triangle_count), sizeof(triangle_count));
  triangles.reserve(triangle_count);

  for (uint32_t i = 0; i < triangle_count; i++) {
    if (!ifs.good()) {
      std::cerr << "broken file: " << filename_ << std::endl;
      return false;
    }
    ifs.read(reinterpret_cast<char *>(&buffer), sizeof(StlTriangle));
    triangles.push_back(buffer);
  }

  if (app_->Connect("zigen-0") == false) return false;

  object_ = new StlObject(app_, triangles, glm::vec3(0.25f));

  return app_->Run();
}
