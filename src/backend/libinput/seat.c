#include <libinput.h>
#include <zen-libinput.h>
#include <zen-util.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
struct zn_libinput {};
#pragma GCC diagnostic pop

ZN_EXPORT struct zn_libinput *
zn_libinput_create(struct udev *udev)
{
  struct zn_libinput *self;
  UNUSED(udev);

  self = zalloc(sizeof *self);
  if (self == NULL) goto err;

  return self;

err:
  return NULL;
}

ZN_EXPORT void
zn_libinput_destroy(struct zn_libinput *self)
{
  free(self);
}
