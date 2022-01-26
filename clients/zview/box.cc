#include <curl/curl.h>
#include <unistd.h>
#include <zukou.h>

#include <iostream>

#include "internal.h"

class BoxDndTask : public zukou::DndTask
{
 public:
  BoxDndTask(Box* box) : box_(box){};
  void Done() { box_->DndFdReady(fd); };

 private:
  Box* box_;
};

Box::Box(App* app, glm::vec3 half_size)
    : zukou::CuboidWindow(app, half_size), application_(app)
{}

void
Box::SetObject(std::unique_ptr<Drawable> object)
{
  object_ = move(object);
  object_->Draw(this);
}

void
Box::Configure(uint32_t serial, glm::vec3 half_size, glm::quat quaternion)
{
  zukou::CuboidWindow::Configure(serial, half_size, quaternion);
  if (object_) object_->Draw(this);
}

void
Box::RayButton(uint32_t serial, uint32_t time, uint32_t button,
    enum zgn_ray_button_state state)
{
  (void)time;
  (void)button;
  if (state == ZGN_RAY_BUTTON_STATE_PRESSED) {
    zgn_cuboid_window_move(cuboid_window(), app()->seat(), serial);
  }
}

void
Box::DataDeviceEnter(uint32_t serial, [[maybe_unused]] glm::vec3 origin,
    [[maybe_unused]] glm::vec3 direction,
    [[maybe_unused]] struct zgn_data_offer* id)
{
  data_device_enter_serial_ = serial;

  if (app()->data_offer() == NULL) return;

  app()->data_offer()->SetActions(ZGN_DATA_DEVICE_MANAGER_DND_ACTION_MOVE |
                                      ZGN_DATA_DEVICE_MANAGER_DND_ACTION_COPY,
      ZGN_DATA_DEVICE_MANAGER_DND_ACTION_COPY);
}

void
Box::DataDeviceMotion([[maybe_unused]] uint32_t time,
    [[maybe_unused]] glm::vec3 origin, [[maybe_unused]] glm::vec3 direction)
{
  if (app()->data_offer() == NULL) return;

  for (auto mime_type : *app()->data_offer()->mime_types()) {
    if (mime_type == "text/x-moz-url") {
      app()->data_offer()->Accept(data_device_enter_serial_, mime_type);
      return;
    }
  }
}

void
Box::DataDeviceDrop()
{
  if (app()->data_offer() == NULL) return;

  for (auto mime_type : *app()->data_offer()->mime_types()) {
    if (mime_type == "text/x-moz-url") {
      BoxDndTask* task = new BoxDndTask(this);
      app()->data_offer()->Receive(mime_type, task);
      return;
    }
  }
}

void
Box::DndFdReady(int fd)
{
  char c;
  std::string str;
  std::string path;
  CURL* curl = curl_easy_init();

  if (!curl) return;

  while (read(fd, &c, 1) > 0) str.push_back(c);

  close(fd);

  if (str.find("file://") == 0) {
    str = str.substr(7);
    int decodelen;
    char* decoded =
        curl_easy_unescape(curl, str.c_str(), str.size(), &decodelen);
    path = std::string(decoded, decodelen);
    application_->UpdateLocation(path);
    curl_free(decoded);
  }

  curl_easy_cleanup(curl);
}
