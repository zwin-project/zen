#define _GNU_SOURCE 1

#include <libzen-compositor/libzen-compositor.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

WL_EXPORT
int
create_shared_file(off_t size, void* content)
{
  const char* name = "zazen-shared";
  int fd;
  void* data;

  fd = memfd_create(name, MFD_CLOEXEC | MFD_ALLOW_SEALING);
  if (fd < 0) return fd;
  unlink(name);

  if (ftruncate(fd, size) < 0) {
    close(fd);
    return -1;
  }

  data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED) {
    close(fd);
    return -1;
  }

  memcpy(data, content, size);

  munmap(data, size);

  return fd;
}
