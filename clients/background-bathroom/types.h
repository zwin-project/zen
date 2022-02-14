#ifndef ZEN_CLIENT_BACKGROUND_ROOM_TYPES_H
#define ZEN_CLIENT_BACKGROUND_ROOM_TYPES_H

#include <glm/glm.hpp>

struct Vertex {
  Vertex(glm::vec3 point, glm::vec3 norm, glm::vec2 texture)
  {
    this->point = point;
    this->norm = norm;
    this->texture = texture;
  };

  glm::vec3 point;
  glm::vec3 norm;
  glm::vec2 texture;
};
#endif  //  ZEN_CLIENT_BACKGROUND_ROOM_TYPES_H
