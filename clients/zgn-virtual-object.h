#ifndef ZGN_CLIENT_VIRTUAL_OBJECT_H
#define ZGN_CLIENT_VIRTUAL_OBJECT_H

#include <assert.h>
#include <wayland-client.h>
#include <zigen-client-protocol.h>

template <class T>
class ZgnVirtualObject
{
 public:
  ZgnVirtualObject(T *delegate, struct zgn_compositor *compositor);
  ~ZgnVirtualObject();
  void NextFrame();
  inline T *delegate();
  inline struct zgn_compositor *compositor();
  inline struct zgn_virtual_object *virtual_object();
  inline struct wl_callback *frame_callback();

 private:
  T *delegate_;
  struct zgn_compositor *compositor_;
  struct zgn_virtual_object *virtual_object_;
  struct wl_callback *frame_callback_;
};

template <class T>
static void
frame_callback_handler(
    void *data, struct wl_callback *callback, uint32_t callback_data)
{
  ZgnVirtualObject<T> *virtual_object = (ZgnVirtualObject<T> *)data;
  wl_callback_destroy(callback);
  virtual_object->delegate()->Frame(callback_data);
}

template <class T>
static const struct wl_callback_listener frame_callback_listener = {
    frame_callback_handler<T>,
};

template <class T>
ZgnVirtualObject<T>::ZgnVirtualObject(
    T *delegate, struct zgn_compositor *compositor)
{
  delegate_ = delegate;
  compositor_ = compositor;
  virtual_object_ = zgn_compositor_create_virtual_object(compositor_);
  frame_callback_ = nullptr;
}

template <class T>
ZgnVirtualObject<T>::~ZgnVirtualObject()
{
  if (frame_callback_) wl_callback_destroy(frame_callback_);
  zgn_virtual_object_destroy(virtual_object_);
}

template <class T>
void
ZgnVirtualObject<T>::NextFrame()
{
  frame_callback_ = zgn_virtual_object_frame(virtual_object_);
  wl_callback_add_listener(frame_callback_, &frame_callback_listener<T>, this);
  zgn_virtual_object_commit(virtual_object_);
}

template <class T>
inline T *
ZgnVirtualObject<T>::delegate()
{
  return delegate_;
}

template <class T>
inline struct zgn_compositor *
ZgnVirtualObject<T>::compositor()
{
  return compositor_;
}

template <class T>
inline struct zgn_virtual_object *
ZgnVirtualObject<T>::virtual_object()
{
  return virtual_object_;
}

template <class T>
inline struct wl_callback *
ZgnVirtualObject<T>::frame_callback()
{
  return frame_callback_;
}

#endif  //  ZGN_CLIENT_VIRTUAL_OBJECT_H
