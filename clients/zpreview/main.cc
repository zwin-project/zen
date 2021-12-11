#include <iostream>

#include "app.h"

void
PrintHelp([[maybe_unused]] int argc, char const *argv[])
{
  std::cerr << "Usage:\t" << argv[0] << " [FILE]" << std::endl;
}

int
main(int argc, char const *argv[])
{
  int exit_code = EXIT_FAILURE;

  if (argc != 2) {
    PrintHelp(argc, argv);
    return exit_code;
  }

  std::string filename(argv[1]);

  App *app = new App(filename);

  if (app->Init() == false) goto out;

  if (app->Show() == false) goto out;

  exit_code = EXIT_SUCCESS;

out:

  delete app;

  return exit_code;
}
