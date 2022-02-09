#ifndef ZEN_CLIENT_BACKGROUND_TYPES_H
#define ZEN_CLIENT_BACKGROUND_TYPES_H

#include <glm/glm.hpp>

struct Vertex {
  Vertex(glm::vec3 point, glm::vec3 norm)
  {
    this->point = point;
    this->norm = norm;
  };

  glm::vec3 point;
  glm::vec3 norm;
};

#endif  //  ZEN_CLIENT_BACKGROUND_TYPES_H
