#ifndef ZEN_CLIENT_BACKGROUND_ROOM_PNG_LOADER_H
#define ZEN_CLIENT_BACKGROUND_ROOM_PNG_LOADER_H

#include <wayland-client.h>

unsigned char *z_helper_png(const char *filename, __uint32_t *width,
    __uint32_t *height, __uint32_t *ch);

class PngLoader
{
 public:
  explicit PngLoader(const char *filename);
  bool Load();
  inline uint32_t width();
  inline uint32_t height();
  inline uint32_t channel();
  inline uint8_t *data();

 private:
  const char *filename_;
  uint32_t width_;
  uint32_t height_;
  uint32_t channel_;
  uint8_t *data_;
};

inline uint32_t
PngLoader::width()
{
  return this->width_;
}
inline uint32_t
PngLoader::height()
{
  return this->height_;
}
inline uint32_t
PngLoader::channel()
{
  return this->channel_;
}
inline uint8_t *
PngLoader::data()
{
  return this->data_;
}

#endif  //  ZEN_CLIENT_BACKGROUND_ROOM_PNG_LOADER_H
