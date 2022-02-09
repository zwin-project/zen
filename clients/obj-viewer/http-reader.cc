#include "http-reader.h"

#include <curl/curl.h>

#include <cstring>
#include <iostream>

size_t
WriteData(char *buffer, size_t size, size_t nmemb, std::vector<char> *buf)
{
  size_t buffer_size = size * nmemb;
  buf->insert(buf->end(), buffer, buffer + buffer_size);
  return buffer_size;
}

HttpReader::HttpReader() { cursor_ = 0; }

bool
HttpReader::Download(std::string url)
{
  CURL *curl;
  CURLcode res;

  curl = curl_easy_init();
  if (curl == NULL) {
    std::cerr << "failed to initialize curl" << std::endl;
    return false;
  }

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf_);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);

  std::cout << "downloading " << url << " ... ";
  std::flush(std::cout);
  res = curl_easy_perform(curl);
  std::cout << buf_.size() << " bytes " << std::endl;

  if (res != CURLE_OK) {
    std::cerr << "failed to get from " << url << ": " << curl_easy_strerror(res)
              << std::endl;
    return false;
  }

  curl_easy_cleanup(curl);

  return true;
}

bool
HttpReader::Read(char *buffer, size_t n)
{
  if (buf_.size() < cursor_ + n) {
    std::memcpy(buffer, buf_.data() + cursor_, buf_.size() - cursor_);
    cursor_ = buf_.size();
    return false;
  }

  std::memcpy(buffer, buf_.data() + cursor_, n);
  cursor_ += n;
  return true;
}

bool
HttpReader::End()
{
  return cursor_ >= buf_.size();
}

bool
HttpReader::GetLine(std::string *str)
{
  str->clear();
  while (cursor_ < buf_.size()) {
    char c = buf_[cursor_];
    cursor_++;
    if (c == '\n') return true;
    str->push_back(c);
  }
  return false;
}
