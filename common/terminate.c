#include "zen-common/terminate.h"

static terminate_func_t terminate_func;
static void *user_data;

void
zn_terminate(int exit_code)
{
  terminate_func(exit_code, user_data);
}

void
zn_set_terminate_func(terminate_func_t terminate, void *data)
{
  terminate_func = terminate;
  user_data = data;
}
