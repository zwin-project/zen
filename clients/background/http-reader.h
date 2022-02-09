#ifndef ZEN_CLIENT_BACKGROUND_HTTP_READER_H
#define ZEN_CLIENT_BACKGROUND_HTTP_READER_H

#include <string>
#include <vector>

#include "reader.h"

class HttpReader : public Reader
{
 public:
  HttpReader();
  bool Download(std::string url);
  bool Read(char *buffer, size_t n);
  bool End();
  bool GetLine(std::string *str);

 private:
  std::vector<char> buf_;
  uint32_t cursor_;
};

#endif  //  ZEN_CLIENT_BACKGROUND_HTTP_READER_H
