#include <iostream>

#include "obj-parser.h"
#include "obj-viewer.h"
#include "zukou.h"

void
PrintHelp([[maybe_unused]] int argc, char const *argv[])
{
  std::cerr << "Usage:\t" << argv[0] << " [OBJ FILE] [MTL FILE] [TEXTURE DIR]"
            << std::endl;
}

int
main([[maybe_unused]] int argc, [[maybe_unused]] char const *argv[])
{
  std::string obj_path, mtl_path, textures_dir_path;
  zukou::App *app = new zukou::App();

  if (argc < 4) {
    PrintHelp(argc, argv);
    return EXIT_FAILURE;
  }
  obj_path = argv[1];
  mtl_path = argv[2];
  textures_dir_path = argv[3];

  ObjParser *parser = new ObjParser(obj_path, mtl_path, textures_dir_path);
  if (!parser->Parse()) {
    std::cerr << "[error] Parsing Obj or Mtl failed!" << std::endl;
    return EXIT_FAILURE;
  }

  if (app->Connect("zigen-0") == false) return EXIT_FAILURE;

  ObjViewer *viewer = new ObjViewer(app, parser);
  (void)viewer;

  if (app->Run())
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}
