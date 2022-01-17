#include <iostream>
#include <string>

#include "dnd.h"
#include "zukou.h"

void
PrintHelp([[maybe_unused]] int argc, char const *argv[])
{
  std::cerr << "Usage:\t" << argv[0] << " [FILENAME].stl" << std::endl;
}

int
main(int argc, char const *argv[])
{
  std::string path = "";
  zukou::App *app = new zukou::App();

  app->Connect("zigen-0");

  if (argc >= 2) {
    path = std::string(argv[1]);

    if (path.rfind(".stl") != path.length() - 4) {
      PrintHelp(argc, argv);
      return EXIT_FAILURE;
    }
  }

  ZDnd *zdnd = new ZDnd(app, path, 0.2f);

  if (zdnd->MainLoop())
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}
