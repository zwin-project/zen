#include "app.h"

int
main()
{
  App *app = new App();

  if (app->Start())
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}
