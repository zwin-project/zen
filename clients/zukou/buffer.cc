#include <sys/mman.h>
#include <unistd.h>
#include <zukou.h>

namespace zukou {

static int
create_shared_fd(off_t size)
{
  const char *name = "zen-simple-box";
  int fd = memfd_create(name, MFD_CLOEXEC | MFD_ALLOW_SEALING);
  if (fd < 0) return fd;
  unlink(name);

  if (ftruncate(fd, size) < 0) {
    close(fd);
    return -1;
  }

  return fd;
}

Buffer::Buffer(App *app, size_t size)
{
  size_ = size;
  fd_ = create_shared_fd(size);
  data_ = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
  if (data_ == MAP_FAILED) {
    close(fd_);
    return;
  }
  pool_ = wl_shm_create_pool(app->shm(), fd_, size);
  wl_buffer_ = wl_shm_pool_create_buffer(pool_, 0, size, 1, size, 0);
}

Buffer::~Buffer()
{
  wl_buffer_destroy(wl_buffer_);
  wl_shm_pool_destroy(pool_);
  munmap(data_, size_);
  close(fd_);
}

}  // namespace zukou
