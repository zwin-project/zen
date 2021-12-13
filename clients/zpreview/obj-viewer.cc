#include "obj-viewer.h"

#include <iostream>
#include <string>
#include <vector>

#include "file-reader.h"
#include "http-reader.h"
#include "reader.h"

ObjViewer::ObjViewer(std::string path)
{
  path_ = path;
  app_ = new zukou::App();
}

ObjViewer::~ObjViewer()
{
  delete app_;
  if (object_) delete object_;
}

void
ObjViewer::Split(std::string *str, std::vector<std::string> *tokens) const
{
  std::string token;
  for (char c : *str) {
    if (c == ' ') {
      if (token.length() > 0) {
        tokens->push_back(token);
        token.clear();
      }
    } else {
      token.push_back(c);
    }
  }

  if (token.length() > 0) tokens->push_back(token);
}

void
ObjViewer::Split(
    std::string *str, std::vector<std::string> *tokens, char delimiter) const
{
  std::string token("");
  for (char c : *str) {
    if (c == delimiter) {
      tokens->push_back(token);
      token.clear();
    } else {
      token.push_back(c);
    }
  }
  tokens->push_back(token);
}

bool
ObjViewer::ParseVertex(std::vector<std::string> *tokens)
{
  if (tokens->size() < 4) return false;
  try {
    glm::vec3 vertex(std::stof(tokens->at(1)), std::stof(tokens->at(2)),
        std::stof(tokens->at(3)));
    vertices_.push_back(vertex);
  } catch (const std::invalid_argument &e) {
    return false;
  } catch (const std::out_of_range &e) {
    return false;
  }
  return true;
}

bool
ObjViewer::ParseNorm(std::vector<std::string> *tokens)
{
  if (tokens->size() < 4) return false;
  try {
    glm::vec3 norm(std::stof(tokens->at(1)), std::stof(tokens->at(2)),
        std::stof(tokens->at(3)));
    norms_.push_back(norm);
  } catch (const std::invalid_argument &e) {
    return false;
  } catch (const std::out_of_range &e) {
    return false;
  }
  return true;
}

bool
ObjViewer::ParseFace(std::vector<std::string> *tokens)
{
  if (tokens->size() < 4) return false;
  ObjFacePoint face_point;
  std::vector<ObjFacePoint> face;
  try {
    for (size_t i = 1; i < tokens->size(); i++) {
      std::vector<std::string> t;
      Split(&tokens->at(i), &t, '/');
      if (t.size() == 1) {
        face_point.vertex_index = t[0] == "" ? 0 : std::stoi(t[0]) - 1;
        face_point.texture_index = 0;
        face_point.norm_index = t[0] == "" ? 0 : std::stoi(t[0]) - 1;
        face.push_back(face_point);
      } else if (t.size() == 3) {
        face_point.vertex_index = t[0] == "" ? 0 : std::stoi(t[0]) - 1;
        face_point.texture_index = t[1] == "" ? 0 : std::stoi(t[1]) - 1;
        face_point.norm_index = t[2] == "" ? 0 : std::stoi(t[2]) - 1;
        face.push_back(face_point);
      } else {
        return false;
      }
    }
  } catch (const std::invalid_argument &e) {
    return false;
  } catch (const std::out_of_range &e) {
    return false;
  }
  faces_.push_back(face);
  return true;
}

bool
ObjViewer::ParseLine(std::string *line)
{
  std::vector<std::string> tokens;
  Split(line, &tokens);
  for (auto token : tokens) {
    if (token == "v") {
      if (!ParseVertex(&tokens)) return false;
    } else if (token == "vn") {
      if (!ParseNorm(&tokens)) return false;
    } else if (token == "f") {
      if (!ParseFace(&tokens)) return false;
    }
  }
  return true;
}

bool
ObjViewer::Show()
{
  Reader *reader;
  std::string line;

  if (path_.find("http") == 0) {
    HttpReader *http_reader = new HttpReader();
    if (!http_reader->Download(path_)) return false;
    reader = http_reader;
  } else {
    FileReader *file_reader = new FileReader();
    if (!file_reader->Open(path_)) return false;
    reader = file_reader;
  }

  while (reader->GetLine(&line))
    if (!ParseLine(&line)) {
      std::cerr << "broken file: " << path_ << std::endl;
      return false;
    }

  delete reader;

  if (app_->Connect("zigen-0") == false) return false;

  object_ = new ObjObject(app_, &faces_, &vertices_, &norms_, glm::vec3(0.25));

  std::vector<glm::vec3>().swap(vertices_);
  std::vector<glm::vec3>().swap(norms_);
  std::vector<std::vector<ObjFacePoint>>().swap(faces_);

  return app_->Run();
}
