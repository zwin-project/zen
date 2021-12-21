#include <time.h>
#include <zukou.h>

#include <iostream>

namespace zukou {
Debug::Debug()
{
  fps_.base_ = {0, 0};
  fps_.count_ = 0;
}

void
Debug::PrintFps(int interval_sec)
{
  if (fps_.base_.tv_sec == 0 && fps_.base_.tv_nsec == 0)
    timespec_get(&fps_.base_, TIME_UTC);
  fps_.count_++;

  struct timespec now;
  timespec_get(&now, TIME_UTC);

  if ((now.tv_sec - fps_.base_.tv_sec) * 1000000000 + now.tv_nsec -
          fps_.base_.tv_nsec >
      1000000000 * interval_sec) {
    std::cout << fps_.count_ / interval_sec << " fps" << std::endl;
    fps_.count_ = 0;
    fps_.base_ = now;
  }
}
}  // namespace zukou
