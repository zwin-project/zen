#ifndef LIBZEN_COMPOSIOR_H
#define LIBZEN_COMPOSIOR_H

#include <cglm/cglm.h>
#include <time.h>
#include <wayland-server.h>

#include "debug.h"
#include "helpers.h"
#include "timespec-util.h"

#ifdef __cplusplus
extern "C" {
#endif

// Dependency
//                                       + ------------------- +
// backend --> renderer -------------->  |                     |
//        \            \---> shell --->  | compositor (libzen) |
//         \-------------------------->  |                     |
//                                       + ------------------- +

struct zen_weak_link {
  struct wl_resource* resource;
  struct wl_listener listener;
};

struct zen_compositor {
  struct wl_display* display;
  struct wl_global* global;

  struct wl_signal frame_signal;

  struct zen_seat* seat;
  struct zen_shell_base* shell_base;
  struct zen_renderer* renderer;
  struct zen_backend* backend;

  struct wl_event_source* repaint_timer;
  uint32_t repaint_window_msec;
};

struct zen_data_offer {
  struct wl_resource* resource;
  struct zen_data_source* data_source;
};

struct zen_data_source {
  struct wl_resource* resource;
  struct wl_array mime_type_list;
  struct zen_data_offer* data_offer;
};

struct zen_data_device {
  struct zen_seat* seat;
  struct wl_list resource_list;

  struct zen_data_source* data_source;

  struct wl_resource* focus_resource;  // TODO: weak linkで書き換え
  struct zen_weak_link focus_virtual_object_link;
  // data deviceのfocusがあたってるVO, rayのfocusとはロジックが異なる
};

struct zen_data_device_manager {
  struct wl_display* display;
  struct wl_global* global;
  struct zen_data_device* data_device;
};

struct zen_virtual_object {
  struct zen_compositor* compositor;
  struct wl_resource* resource;

  char* role;
  void* role_object;

  mat4 model_matrix;

  struct wl_signal commit_signal;
  struct wl_signal destroy_signal;
  struct wl_signal render_commit_signal;

  struct wl_listener compositor_frame_signal_listener;
  struct wl_list frame_callback_list;

  struct {
    struct wl_list frame_callback_list;
  } pending;
};

struct zen_ray_motion_event {
  vec3 delta_origin;
  float delta_polar_angle;
  float delta_azimuthal_angle;
};

struct zen_ray_grab {
  const struct zen_ray_grab_interface* interface;
  struct zen_ray* ray;
};

struct zen_ray_grab_interface {
  void (*focus)(struct zen_ray_grab* grab);
  void (*motion)(struct zen_ray_grab* grab, const struct timespec* time,
      struct zen_ray_motion_event* event);
  void (*button)(struct zen_ray_grab* grab, const struct timespec* time,
      uint32_t button, uint32_t state);
  void (*cancel)(struct zen_ray_grab* grab);
};

struct zen_ray {
  struct zen_seat* seat;
  struct zen_ray_grab* grab;
  struct zen_ray_grab default_grab;
  uint32_t button_count;
  uint32_t grab_button;
  uint32_t grab_serial;

  struct zen_weak_link focus_virtual_object_link;

  struct wl_list ray_client_list;
  struct wl_signal destroy_signal;

  vec3 origin;
  struct {
    float polar;      // angle with respect to y axis (0 <= polar < pi * 2)
    float azimuthal;  // (1, 0, 0) -> 0 rad, (0, 0, 1) -> pi/2 rad
                      // (0 <= azimuthal < pi * 2)
  } angle;            // radian

  vec3 local_origin;
  vec3 local_direction;
  float target_distance;

  struct zen_render_item* render_item;
};

struct zen_seat {
  struct wl_global* global;
  struct zen_compositor* compositor;
  struct zen_data_device* data_device;

  struct zen_ray* ray;
  int ray_device_count;

  struct wl_list resource_list;

  char* seat_name;
};

struct zen_shell_base {
  const char* type;
  struct zen_virtual_object* (*pick_virtual_object)(
      struct zen_shell_base* shell_base, struct zen_ray* ray,
      vec3 local_ray_origin, vec3 local_ray_direction, float* distance);
};

struct zen_renderer {
  const char* type;
};

struct zen_render_item {
  void (*commit)(struct zen_render_item* render_item);
};

struct zen_output {
  struct timespec frame_time;

  void (*repaint)(struct zen_output* output);
};

struct zen_backend {
  struct zen_output* output;
};

struct zen_udev_seat;

// methods of zen_compositor
struct zen_compositor* zen_compositor_create(struct wl_display* display);

void zen_compositor_destroy(struct zen_compositor* compositor);

int zen_compositor_load_shell(struct zen_compositor* compositor);

int zen_compositor_load_renderer(struct zen_compositor* compositor);

int zen_compositor_load_backend(struct zen_compositor* compositor);

void zen_compositor_finish_frame(
    struct zen_compositor* compositor, struct timespec next_repaint);

// methods of zen_data_offer
struct zen_data_offer* zen_data_offer_create(
    struct zen_data_source* data_source, struct wl_resource* target);

void zen_data_offer_offer(
    struct zen_data_offer* data_offer, const char* mime_type);

// methods of zen_data_source
struct zen_data_source* zen_data_source_create(
    struct wl_client* client, struct wl_resource* resource, uint32_t id);

void zen_data_source_target(
    struct zen_data_source* data_source, const char* mime_type);

void zen_data_source_send(
    struct zen_data_source* data_source, const char* mime_type, int32_t fd);

void zen_data_source_cancelled(struct zen_data_source* data_source);

void zen_data_source_dnd_drop_performed(struct zen_data_source* data_source);

void zen_data_source_dnd_finished(struct zen_data_source* data_source);

// methods of zen_data_device
struct zen_data_device* zen_data_device_ensure(
    struct wl_client* client, struct zen_seat* seat);

int zen_data_device_add_resource(
    struct zen_data_device* data_device, struct wl_client* client, uint32_t id);

struct wl_resource* zen_data_device_create_insert_resource(
    struct wl_client* client, uint32_t id);

void zen_data_device_data_offer(struct zen_data_device* data_device,
    struct zen_data_offer* data_offer,
    struct zen_virtual_object* virtual_object);

void zen_data_device_enter(struct zen_data_device* data_device,
    struct zen_virtual_object* virtual_object, struct zen_ray* ray,
    struct zen_data_offer* data_offer);

void zen_data_device_leave(struct zen_data_device* data_device);

void zen_data_device_motion(struct zen_data_device* data_device,
    struct zen_ray* ray, const struct timespec* time);

void zen_data_device_drop(struct zen_data_device* data_device);

// methods of zen_data_device_manager
struct zen_data_device_manager* zen_data_device_manager_create(
    struct wl_display* display);

void zen_data_device_manager_destroy(
    struct zen_data_device_manager* data_device_manager);

// methods of zen_ray
void zen_ray_clear_focus(struct zen_ray* ray);

void zen_ray_grab_start(struct zen_ray* ray, struct zen_ray_grab* grab);

void zen_ray_grab_end(struct zen_ray* ray);

void zen_ray_get_direction(struct zen_ray* ray, vec3 direction);

void zen_ray_move(struct zen_ray* ray, struct zen_ray_motion_event* event);

struct zen_ray* zen_ray_create(struct zen_seat* seat);

void zen_ray_destroy(struct zen_ray* ray);

// methods of zen_seat

void zen_seat_notify_add_ray(struct zen_seat* seat);

void zen_seat_notify_release_ray(struct zen_seat* seat);

void zen_seat_notify_add_keyboard(struct zen_seat* seat);

void zen_seat_notify_release_keyboard(struct zen_seat* seat);

void zen_seat_notify_ray_motion(struct zen_seat* seat,
    const struct timespec* time, struct zen_ray_motion_event* event);

void zen_seat_notify_ray_button(struct zen_seat* seat,
    const struct timespec* time, int32_t button, uint32_t state);

void zen_seat_notify_key(struct zen_seat* seat, const struct timespec* time,
    uint32_t key, uint32_t state);

struct zen_seat* zen_seat_create(struct zen_compositor* compositor);

void zen_seat_destroy(struct zen_seat* seat);

// methods of zen_weak_link
void zen_weak_link_init(struct zen_weak_link* link);

void zen_weak_link_set(
    struct zen_weak_link* link, struct wl_resource* resource);

void zen_weak_link_unset(struct zen_weak_link* link);

void* zen_weak_link_get_user_data(struct zen_weak_link* link);

// methods of zen_udev_seat

struct zen_udev_seat* zen_udev_seat_create(struct zen_compositor* compositor);

void zen_udev_seat_destroy(struct zen_udev_seat* udev_seat);

// Interfaces below will be implementaed outside of libzen-compositor

// methods of zen_shell;
struct zen_shell_base* zen_shell_create(struct zen_compositor* compositor);

void zen_shell_destroy(struct zen_shell_base* shell);

// methods of zen_backend
struct zen_backend* zen_backend_create(struct zen_compositor* compositor);

void zen_backend_destroy(struct zen_backend* backend);

// methods of opengl_renderer
struct zen_renderer* zen_renderer_create(struct zen_compositor* compositor);

void zen_renderer_destroy(struct zen_renderer* renderer);

struct zen_render_item* zen_ray_render_item_create(
    struct zen_renderer* renderer, struct zen_ray* ray);

void zen_ray_render_item_destroy(struct zen_render_item* render_item);

#ifdef __cplusplus
}
#endif

#endif  //  LIBZEN_COMPOSIOR_H
