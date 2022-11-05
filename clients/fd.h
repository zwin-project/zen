#pragma once

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

static int
create_anonymous_file(off_t size)
{
  int fd;
  int ret;
#ifdef HAVE_MEMFD_CREATE
  fd = memfd_create("zen-shared", MFD_CLOEXEC | MFD_ALLOW_SEALING);
#else
#error Operating system must support memfd_create
  // TODO: fallback
#endif

#ifdef HAVE_POSIX_FALLOCATE
  do {
    ret = posix_fallocate(fd, 0, size);
  } while (ret == EINTR);
  if (ret != 0) {
    close(fd);
    errno = ret;
    return -1;
  }
#else
  do {
    ret = ftruncate(fd, size);
  } while (ret < 0 && errno == EINTR);
  if (ret < 0) {
    close(fd);
    return -1;
  }
#endif

  return fd;
}
