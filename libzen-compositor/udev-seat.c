#include <errno.h>
#include <fcntl.h>
#include <libinput.h>
#include <libudev.h>
#include <libzen-compositor/libzen-compositor.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <zigen-server-protocol.h>

struct zen_udev_seat {
  struct zen_seat* base;
  struct libinput* libinput;
  struct udev* udev;
  struct wl_event_source* event_source;
};

static int
open_restricted(const char* path, int flags, void* user_data)
{
  UNUSED(user_data);
  int fd = open(path, flags);

  if (fd < 0)
    zen_log("libinput device: failed to open %s (%s)\n", path, strerror(errno));
  return fd < 0 ? -errno : fd;
}

static void
close_restricted(int fd, void* user_data)
{
  UNUSED(user_data);
  close(fd);
}

static const struct libinput_interface interface = {
    .open_restricted = open_restricted,
    .close_restricted = close_restricted,
};

static void
handle_pointer_motion(
    struct zen_seat* seat, struct libinput_event_pointer* pointer_event)
{
  UNUSED(seat);
  UNUSED(pointer_event);
  struct timespec time;
  struct zen_ray_motion_event event = {0};

  timespec_from_usec(
      &time, libinput_event_pointer_get_time_usec(pointer_event));

  event.delta_polar_angle = libinput_event_pointer_get_dy(pointer_event) / 1024;
  event.delta_azimuthal_angle =
      libinput_event_pointer_get_dx(pointer_event) / 1024;

  zen_seat_notify_ray_motion(seat, &time, &event);
}

static void
handle_pointer_button(
    struct zen_seat* seat, struct libinput_event_pointer* pointer_event)
{
  struct timespec time;
  int button_state = libinput_event_pointer_get_button_state(pointer_event);
  int seat_button_count =
      libinput_event_pointer_get_seat_button_count(pointer_event);

  if ((button_state == LIBINPUT_BUTTON_STATE_PRESSED &&
          seat_button_count != 1) ||
      (button_state == LIBINPUT_BUTTON_STATE_RELEASED &&
          seat_button_count != 0))
    return;

  timespec_from_usec(
      &time, libinput_event_pointer_get_time_usec(pointer_event));

  zen_seat_notify_ray_button(seat, &time,
      libinput_event_pointer_get_button(pointer_event), button_state);
}

static void
handle_keyboard_key(
    struct zen_seat* seat, struct libinput_event_keyboard* keyboard_event)
{
  UNUSED(seat);
  UNUSED(keyboard_event);
  struct timespec time;
  uint32_t key_state = libinput_event_keyboard_get_key_state(keyboard_event);
  uint32_t seat_key_count =
      libinput_event_keyboard_get_seat_key_count(keyboard_event);
  uint32_t key = libinput_event_keyboard_get_key(keyboard_event);

  if ((key_state == LIBINPUT_KEY_STATE_PRESSED && seat_key_count != 1) ||
      (key_state == LIBINPUT_KEY_STATE_RELEASED && seat_key_count != 0))
    return;

  timespec_from_usec(
      &time, libinput_event_keyboard_get_time_usec(keyboard_event));

  zen_seat_notify_key(seat, &time, key, key_state);
}

static void
handle_device_added(struct zen_seat* seat, struct libinput_device* device)
{
  // TODO: Create device objects and manage the devices better (refer to
  // weston).
  if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_KEYBOARD)) {
    zen_seat_notify_add_keyboard(seat);
  }

  if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_POINTER)) {
    zen_seat_notify_add_ray(seat);
  }
}

static void
handle_device_removed(struct zen_seat* seat, struct libinput_device* device)
{
  if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_KEYBOARD)) {
    zen_seat_notify_release_keyboard(seat);
  }

  if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_POINTER)) {
    zen_seat_notify_release_ray(seat);
  }
}

static int
handle_event(int fd, uint32_t mask, void* data)
{
  UNUSED(fd);
  UNUSED(mask);
  struct zen_udev_seat* udev_seat = data;
  struct libinput_event* event;

  if (libinput_dispatch(udev_seat->libinput))
    zen_log("libinput device: failed to dispatch libinput\n");

  while ((event = libinput_get_event(udev_seat->libinput))) {
    switch (libinput_event_get_type(event)) {
      case LIBINPUT_EVENT_POINTER_MOTION:
        handle_pointer_motion(
            udev_seat->base, libinput_event_get_pointer_event(event));
        break;

      case LIBINPUT_EVENT_POINTER_BUTTON:
        handle_pointer_button(
            udev_seat->base, libinput_event_get_pointer_event(event));
        break;

      case LIBINPUT_EVENT_KEYBOARD_KEY:
        handle_keyboard_key(
            udev_seat->base, libinput_event_get_keyboard_event(event));
        break;

      case LIBINPUT_EVENT_DEVICE_ADDED:
        handle_device_added(udev_seat->base, libinput_event_get_device(event));
        break;

      case LIBINPUT_EVENT_DEVICE_REMOVED:
        handle_device_removed(
            udev_seat->base, libinput_event_get_device(event));
        break;

      default:
        break;
    }
    libinput_event_destroy(event);
  }

  return 0;
}

WL_EXPORT struct zen_udev_seat*
zen_udev_seat_create(struct zen_compositor* compositor)
{
  struct zen_udev_seat* udev_seat;
  struct wl_event_source* event_source;
  struct libinput* libinput;
  struct udev* udev;
  int fd;

  udev_seat = zalloc(sizeof *udev_seat);
  if (udev_seat == NULL) {
    zen_log("libinput device: failed to allocate memory\n");
    goto err;
  }

  udev = udev_new();
  if (udev == NULL) {
    zen_log("libinput device: failed to initialize udev\n");
    goto err_udev;
  }

  libinput = libinput_udev_create_context(&interface, udev_seat, udev);
  if (libinput == NULL) {
    zen_log("libinput device: failed to create a libinput udev context\n");
    goto err_libinput;
  }

  if (libinput_udev_assign_seat(libinput, compositor->seat->seat_name) != 0) {
    zen_log("libinput device: failed to assign seat\n");
    goto err_assign_seat;
  }

  fd = libinput_get_fd(libinput);
  event_source =
      wl_event_loop_add_fd(wl_display_get_event_loop(compositor->display), fd,
          WL_EVENT_READABLE, handle_event, udev_seat);

  udev_seat->base = compositor->seat;
  udev_seat->udev = udev;
  udev_seat->libinput = libinput;
  udev_seat->event_source = event_source;

  return udev_seat;

err_assign_seat:
  libinput_unref(libinput);

err_libinput:
  udev_unref(udev);

err_udev:
  free(udev_seat);

err:
  return NULL;
}

WL_EXPORT void
zen_udev_seat_destroy(struct zen_udev_seat* udev_seat)
{
  wl_event_source_remove(udev_seat->event_source);
  libinput_unref(udev_seat->libinput);
  udev_unref(udev_seat->udev);
  free(udev_seat);
}
