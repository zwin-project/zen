#ifndef ZEN_CLIENT_BACKGROUND_FILE_READER_H
#define ZEN_CLIENT_BACKGROUND_FILE_READER_H

#include <fstream>
#include <string>

#include "reader.h"

class FileReader : public Reader
{
 public:
  FileReader();
  bool Open(std::string path);
  bool Read(char *buffer, size_t n);
  bool End();
  bool GetLine(std::string *str);

 private:
  std::ifstream ifs_;
};

#endif  //  ZEN_CLIENT_BACKGROUND_FILE_READER_H
