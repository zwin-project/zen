#include "zen-common/log.h"
#include "zen-common/terminate.h"
#include "zen-common/util.h"

static void
zn_terminate_func(int /*exit_code*/, void * /*data*/)
{}

auto
main(int /*argc*/, char * /*argv*/[]) -> int
{
  zn_set_terminate_func(zn_terminate_func, nullptr);
  zn_log_init(ZEN_DEBUG, zn_terminate);

  zn_info("Hello Zen!!!");

  return 1;
}
