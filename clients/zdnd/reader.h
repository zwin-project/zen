#ifndef ZEN_CLIENT_ZDND_READER_H
#define ZEN_CLIENT_ZDND_READER_H

#include <string>

class Reader
{
 public:
  virtual ~Reader(){};
  virtual bool Read([[maybe_unused]] char *buffer, [[maybe_unused]] size_t n)
  {
    return false;
  };
  virtual bool End() { return false; };
  virtual bool GetLine([[maybe_unused]] std::string *str) { return false; };
};

#endif  //  ZEN_CLIENT_ZDND_READER_H
