#pragma once

#include <zen-common.h>
#include <zigen-client-protocol.h>

#include <memory>

namespace zen::client {

class Application;

class VirtualObject
{
 public:
  DISABLE_MOVE_AND_COPY(VirtualObject);
  VirtualObject() = delete;
  VirtualObject(Application *app);
  virtual ~VirtualObject();

  bool Init();

  void Commit();

  void NextFrame(void);

  inline zgn_virtual_object *proxy();

 protected:
  virtual void Frame(uint32_t /*time*/){};

 private:
  static const struct wl_callback_listener callback_listener_;

  static void Callback(
      void *data, struct wl_callback *wl_callback, uint32_t callback_data);

  Application *app_;
  zgn_virtual_object *proxy_ = nullptr;  // nonnull after initialization
  wl_callback *callback_ = nullptr;      // nullable
};

inline zgn_virtual_object *
VirtualObject::proxy()
{
  return proxy_;
}

}  // namespace zen::client
