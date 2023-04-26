#pragma once

struct zn_buffer;

#ifdef __cplusplus
extern "C" {
#endif

// NOLINTNEXTLINE(modernize-use-using)
typedef void (*zn_lease_buffer_release_callback)(
    struct zn_buffer *buffer, void *user_data);

struct zn_lease_buffer {
  void *user_data;                            // @nullable, @outlive
  struct zn_buffer *buffer;                   // @nonnull, @outlive
  zn_lease_buffer_release_callback callback;  // @nullable, @outlive
};

/// @param zn_buffer must outlive this
/// @param callback is nullable
struct zn_lease_buffer *zn_lease_buffer_create(struct zn_buffer *buffer,
    zn_lease_buffer_release_callback callback, void *user_data);

/// This also destroy itself
void zn_lease_buffer_release(struct zn_lease_buffer *self);

#ifdef __cplusplus
}
#endif
