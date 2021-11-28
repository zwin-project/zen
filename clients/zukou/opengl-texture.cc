
#include <zukou.h>

namespace zukou {
OpenGLTexture::OpenGLTexture(App *app, int32_t height, int32_t width)
    : Buffer(app, width * 4, height, width, WL_SHM_FORMAT_ARGB8888)
{
  texture_ = zgn_opengl_create_texture(app->opengl());
}

void
OpenGLTexture::BufferUpdated()
{
  zgn_opengl_texture_attach_2d(texture_, wl_buffer_);
}
}  // namespace zukou
