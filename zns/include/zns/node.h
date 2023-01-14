#pragma once

#include <cglm/vec3.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_pointer.h>
#include <zwin-protocol.h>

struct zns_node_interface {
  /**
   * @returns true if intersected
   * @param distance stores the minimum collision distance found so far.
   * If the node detect the intersection with a collision distance shorter than
   * `distance`, it overrides `distance` with the collision distance.
   */
  bool (*ray_cast)(void *user_data, vec3 origin, vec3 direction, mat4 transform,
      float *distance);

  /**
   * @return false to pass the event to the parent node
   */
  bool (*ray_motion)(
      void *user_data, vec3 origin, vec3 direction, uint32_t time_msec);

  /**
   * @return false to pass the event to the parent node
   */
  bool (*ray_enter)(void *user_data, vec3 origin, vec3 direction);

  /**
   * @return false to pass the event to the parent node
   */
  bool (*ray_leave)(void *user_data);

  /**
   * @return false to pass the event to the parent node
   */
  bool (*ray_button)(void *user_data, uint32_t time_msec, uint32_t button,
      enum wlr_button_state state);

  /**
   * @return false to pass the event to the parent node
   */
  bool (*ray_axis)(void *user_data, uint32_t time_msec,
      enum wlr_axis_source source, enum wlr_axis_orientation orientation,
      double delta, int32_t delta_discrete);

  /**
   * @return false to pass the event to the parent node
   */
  bool (*ray_frame)(void *user_data);
};

struct zns_node {
  struct zns_node *parent;
  struct wl_list children;  // zns_node::link;
  struct wl_list link;      // zns_node::children

  void *user_data;
  const struct zns_node_interface *implementation;

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  mat4 transform;  // translation and rotation only
};

/**
 * @returns intersected node
 * @param distance stores the minimum collision distance found so far.
 * If the node detect the intersection with a collision distance shorter than
 * `distance`, it overrides `distance` to with the collision distance and
 * returns the intersected node.
 */
struct zns_node *zns_node_ray_cast(struct zns_node *self, vec3 origin,
    vec3 direction, mat4 parent_transform, float *distance);

void zns_node_ray_motion(
    struct zns_node *self, vec3 origin, vec3 direction, uint32_t time_msec);

void zns_node_ray_enter(struct zns_node *self, vec3 origin, vec3 direction);

void zns_node_ray_leave(struct zns_node *self);

void zns_node_ray_button(struct zns_node *self, uint32_t time_msec,
    uint32_t button, enum wlr_button_state state);

void zns_node_ray_axis(struct zns_node *self, uint32_t time_msec,
    enum wlr_axis_source source, enum wlr_axis_orientation orientation,
    double delta, int32_t delta_discrete);

void zns_node_ray_frame(struct zns_node *self);

/**
 * @param parent can be null only for root node
 */
struct zns_node *zns_node_create(struct zns_node *parent, void *user_data,
    const struct zns_node_interface *implementation);

void zns_node_destroy(struct zns_node *self);
