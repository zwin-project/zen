#pragma once

#include <wayland-server-core.h>
#include <znr/buffer.h>
#include <znr/remote.h>

struct zn_remote_mem_buffer {
  void* data;
  size_t size;
  uint32_t ref;

  struct znr_buffer* znr_buffer;
  struct wl_listener znr_buffer_ref_listener;
  uint32_t ref_internal;
};

/**
 * Increment the reference count of buffer by one.
 */
void zn_remote_mem_buffer_ref(struct zn_remote_mem_buffer* self);

/**
 * Decrement the reference count of buffer by one.
 */
void zn_remote_mem_buffer_unref(struct zn_remote_mem_buffer* self);

/**
 * Create buffer in memory space.
 *
 * The buffer is refcounted. Then the reference count reaches 0, the buffer
 * becomes invalid. When the buffer becomes unused, event inside `znr`
 * (ref_internal == 0), the buffer is actually destroyed.
 *
 * The reference count is zero at the time of creation, so call
 * `zn_remote_mem_buffer_ref` immediately for continuous use.
 */
struct zn_remote_mem_buffer* zn_remote_mem_buffer_create(
    size_t size, struct znr_remote* remote);
