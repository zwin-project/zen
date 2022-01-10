#include "box.h"
#include "cmdline.h"
#include "zukou.h"

int
main(int argc, char *argv[])
{
  cmdline::parser cmd;

  cmd.add("fps", '\0', "print fps");
  cmd.parse_check(argc, argv);

  zukou::App *app = new zukou::App();

  app->Connect("zigen-0");

  Box *box = new Box(app, 0.2f, cmd.exist("fps"));
  box->NextFrame();

  if (app->Run())
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}
