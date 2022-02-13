#include <iostream>

#include "field.h"
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

  app->Connect("zigen-0");

  Field *field = new Field(app);
  field->NextFrame();

  if (app->Run())
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}
