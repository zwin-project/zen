#include "file-reader.h"

FileReader::FileReader() {}

bool
FileReader::Open(std::string path)
{
  ifs_.open(path);
  return !ifs_.fail();
}

bool
FileReader::Read(char *buffer, size_t n)
{
  ifs_.read(buffer, n);
  return ifs_.good();
}

bool
FileReader::End()
{
  return ifs_.eof();
}

bool
FileReader::GetLine(std::string *str)
{
  return getline(ifs_, *str).good();
}
