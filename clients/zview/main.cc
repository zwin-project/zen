#include <iostream>
#include <string>
#include <vector>

#include "cmdline.h"
#include "internal.h"

int
main(int argc, char const* argv[])
{
  int exit_code = EXIT_FAILURE;
  cmdline::parser cmd;
  std::string location;
  App* app;

  cmd.add("help", 'h', "print help");

  if (cmd.parse(argc, argv) == false) {
    std::cout << cmd.error_full() << cmd.usage();
    goto err;
  }

  if (cmd.exist("help")) {
    std::cout << cmd.usage();
    goto out;
  }

  if (cmd.rest().size() == 0)
    location = "";
  else
    location = cmd.rest().at(0);

  app = new App();
  app->set_location(location);

  if (app->Show() == false) goto err;

out:
  exit_code = EXIT_SUCCESS;

err:
  delete app;

  return exit_code;
}
