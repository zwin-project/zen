#include "box.h"
#include "zukou.h"

int
main()
{
  zukou::App *app = new zukou::App();

  app->Connect("zigen-0");

  Box *box = new Box(app, 0.2f);
  box->NextFrame();

  if (app->Run())
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}
