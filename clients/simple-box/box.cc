#include "box.h"

#include <time.h>
#include <zukou.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>

struct Line {
  glm::vec3 s, e;
};

Box::Box(zukou::App *app, float length)
    : CuboidWindow(app, glm::vec3(length * 1.8))
{
  srand(time(0));
  length_ = length;
  rx_ = 0;
  ry_ = 0;
  component_ = new zukou::OpenGLComponent(app, this);
  vertex_buffer_ = new zukou::OpenGLVertexBuffer(app, sizeof(Line) * 12);
  Line *edges = (Line *)vertex_buffer()->data();
  for (int i = 0; i < 3; i++) {
    int x = i;
    int y = (i + 1) % 3;
    int z = (i + 2) % 3;
    for (int j = -1; j < 2; j += 2) {
      for (int k = -1; k < 2; k += 2) {
        edges->s[x] = -length_;
        edges->s[y] = length_ * j;
        edges->s[z] = length_ * k;
        edges->e[x] = length_;
        edges->e[y] = length_ * j;
        edges->e[z] = length_ * k;
        edges++;
      }
    }
  }
}

void
Box::Frame(uint32_t time)
{
  (void)time;
  rx_ += (float)(rand() - RAND_MAX / 2) / (float)RAND_MAX;
  rx_ = rx_ > 10 ? 10 : rx_ < -10 ? -10 : rx_;
  ry_ += (float)(rand() - RAND_MAX / 2) / (float)RAND_MAX;
  ry_ = ry_ > 10 ? 10 : ry_ < -10 ? -10 : ry_;

  Line *edge = (Line *)vertex_buffer()->data();
  for (int i = 0; i < 12; i++) {
    edge->s = glm::rotateY(edge->s, glm::pi<float>() * 0.001f * ry_);
    edge->e = glm::rotateY(edge->e, glm::pi<float>() * 0.001f * ry_);
    edge->s = glm::rotateX(edge->s, glm::pi<float>() * 0.001f * rx_);
    edge->e = glm::rotateX(edge->e, glm::pi<float>() * 0.001f * rx_);
    edge++;
  }
  vertex_buffer()->BufferUpdated();
  component()->Attach(vertex_buffer());
  this->NextFrame();
}
