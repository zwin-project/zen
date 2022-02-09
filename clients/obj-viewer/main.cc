#include <iostream>

#include "obj-parser.h"
#include "obj-viewer.h"
#include "zukou.h"

void
PrintHelp([[maybe_unused]] int argc, char const *argv[])
{
  std::cerr << "Usage:\t" << argv[0] << " [OBJ FILE] [MTL FILE]" << std::endl;
}

int
main(int argc, char const *argv[])
{
  (void)argc;
  (void)argv;
  zukou::App *app = new zukou::App();
  if (argc < 2) {
    PrintHelp(argc, argv);
    return EXIT_FAILURE;
  }

  std::string obj_path(argv[1]);
  std::string mtl_path;

  if (argc == 3)
    mtl_path = argv[2];
  else
    mtl_path = "";

  ObjParser *parser = new ObjParser(obj_path, mtl_path);
  if (!parser->Parse()) {
    std::cerr << "Parsing Obj or Mtl failed!" << std::endl;
    return EXIT_FAILURE;
  }

  if (app->Connect("zigen-0") == false) return EXIT_FAILURE;

  ObjViewer *viewer = new ObjViewer(app, parser);
  viewer->Render();

  if (app->Run())
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}
