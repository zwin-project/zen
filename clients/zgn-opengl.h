#ifndef ZGN_CLIENT_OPENGL_H
#define ZGN_CLIENT_OPENGL_H

#include <zigen-opengl-client-protocol.h>

class ZgnOpenGL
{
 public:
  ZgnOpenGL(struct zgn_opengl *opengl) : opengl_(opengl){};
  ~ZgnOpenGL();

  struct zgn_opengl *opengl();

 private:
  struct zgn_opengl *opengl_;
};

inline struct zgn_opengl *
ZgnOpenGL::opengl()
{
  return opengl_;
}

#endif  //  ZGN_CLIENT_OPENGL_H
