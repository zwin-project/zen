#ifndef ZEN_CLIENT_BACKGROUND_FIELD_H
#define ZEN_CLIENT_BACKGROUND_FIELD_H

#include <zukou.h>

class Field : public zukou::Background
{
 public:
  Field(zukou::App *app);

 private:
  zukou::OpenGLComponent *sky_component_;
  zukou::OpenGLShaderProgram *sky_shader_;
};

#endif  // ZEN_CLIENT_BACKGROUND_FIELD_H
