#include "field.h"
#include "zukou.h"

int
main(void)
{
  zukou::App *app = new zukou::App();

  app->Connect("zigen-0");

  Field *field = new Field(app);
  field->Commit();

  if (app->Run())
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}
