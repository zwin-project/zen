#include "field.h"
#include "zukou.h"

int
main(void)
{
  zukou::App *app = new zukou::App();

  app->Connect("zigen-0");

  // Box *box = new Box(app, 0.2f, cmd.exist("fps"));
  // box->NextFrame();
  Field *field = new Field(app);
  field->Commit();

  if (app->Run())
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}
