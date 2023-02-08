#include "zen-common/terminate.h"

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static zn_terminate_func_t terminate_func;
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static void *user_data;

void
zn_terminate(int exit_code)
{
  terminate_func(exit_code, user_data);
}

void
zn_set_terminate_func(zn_terminate_func_t terminate, void *data)
{
  terminate_func = terminate;
  user_data = data;
}
