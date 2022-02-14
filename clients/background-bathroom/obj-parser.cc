#include "obj-parser.h"

#include <dirent.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "file-reader.h"
#include "http-reader.h"
#include "reader.h"

ObjParser::ObjParser(
    std::string obj_path, std::string mtl_path, std::string textures_dir_path)
    : obj_path_(obj_path),
      mtl_path_(mtl_path),
      textures_dir_path_(textures_dir_path)
{
  target_mtl_path_ = "";
  pending_mtl_name_ = "";
  if (textures_dir_path_[textures_dir_path_.size() - 1] != '/')
    textures_dir_path_ += "/";
}

ObjParser::~ObjParser() {}

void
ObjParser::Split(std::string *str, std::vector<std::string> *tokens) const
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
ObjParser::Split(
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

std::string
ObjParser::ExtractFileName(std::string *path)
{
  std::string file_name("");
  for (char c : *path) {
    if (c == '/')
      file_name.clear();
    else
      file_name.push_back(c);
  }
  return file_name;
}

bool
ObjParser::ParseMtlPath(std::vector<std::string> *tokens)
{
  if (tokens->size() < 2) return false;

  target_mtl_path_ = tokens->at(1);
  return true;
}

bool
ObjParser::ParseObjName(std::vector<std::string> *tokens)
{
  if (tokens->size() < 2) return false;

  ObjObject obj;
  obj.name = tokens->at(1);
  obj.smooth_shading = false;

  obj_list_.push_back(obj);

  return true;
}

bool
ObjParser::ParseVertex(std::vector<std::string> *tokens)
{
  if (tokens->size() < 4) return false;
  if (obj_list_.size() == 0) return false;

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
ObjParser::ParseTextureVertex(std::vector<std::string> *tokens)
{
  if (tokens->size() < 3) return false;
  if (obj_list_.size() == 0) return false;

  try {
    glm::vec2 texture_vertex(
        std::stof(tokens->at(1)), std::stof(tokens->at(2)));
    texture_vertices_.push_back(texture_vertex);
  } catch (const std::invalid_argument &e) {
    return false;
  } catch (const std::out_of_range &e) {
    return false;
  }
  return true;
}

bool
ObjParser::ParseNorm(std::vector<std::string> *tokens)
{
  if (tokens->size() < 4) return false;
  if (obj_list_.size() == 0) return false;

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
ObjParser::ParseObjMtlName(std::vector<std::string> *tokens)
{
  if (tokens->size() < 2) return false;
  if (obj_list_.size() == 0) return false;

  pending_mtl_name_ = tokens->at(1);

  return true;
}

bool
ObjParser::ParseSmoothShading(std::vector<std::string> *tokens)
{
  if (tokens->size() < 2) return false;
  if (obj_list_.size() == 0) return false;

  if (tokens->at(1) == "off")
    obj_list_[obj_list_.size() - 1].smooth_shading = false;

  return true;
}

bool
ObjParser::ParseFace(std::vector<std::string> *tokens)
{
  if (tokens->size() < 4) return false;
  if (obj_list_.size() == 0) return false;
  if (pending_mtl_name_ == "") return false;
  ObjFacePoint face_point;
  std::vector<ObjFacePoint> face;

  try {
    for (size_t i = 1; i < tokens->size(); i++) {
      std::vector<std::string> t;
      Split(&tokens->at(i), &t, '/');
      if (t.size() == 1) {
        face_point.vertex_index = t[0] == "" ? 0 : std::stoi(t[0]);
        face_point.texture_index = 0;
        face_point.norm_index = t[0] == "" ? 0 : std::stoi(t[0]);
        face.push_back(face_point);
      } else if (t.size() == 3) {
        face_point.vertex_index = t[0] == "" ? 0 : std::stoi(t[0]);
        face_point.texture_index = t[1] == "" ? 0 : std::stoi(t[1]);
        face_point.norm_index = t[2] == "" ? 0 : std::stoi(t[2]);
        face.push_back(face_point);

        if (face_point.vertex_index >= (int)vertices_.size()) {
          std::cerr << "[error] vertex index is out of range. At the line, the "
                       "vertex index is greater than the number of vertices"
                    << std::endl;
          return false;
        }
        if (face_point.texture_index >= (int)texture_vertices_.size()) {
          std::cerr
              << "[error] texture index is out of range. At the line, the "
                 "texture index is greater than the number of texture vertices"
              << std::endl;
          return false;
        }
        if (face_point.norm_index >= (int)norms_.size()) {
          std::cerr << "[error] norm index is out of range. At the line, the "
                       "norm index is greater than the number of norm vertices"
                    << std::endl;
          return false;
        }
      } else {
        return false;
      }
    }
  } catch (const std::invalid_argument &e) {
    return false;
  } catch (const std::out_of_range &e) {
    return false;
  }

  for (size_t l = 2; l < face.size(); l++) {
    face_point = face[0];
    obj_list_[obj_list_.size() - 1].face_table[pending_mtl_name_].push_back(
        Vertex(vertices_[face_point.vertex_index],
            norms_[face_point.norm_index],
            texture_vertices_[face_point.texture_index]));

    face_point = face[l - 1];
    obj_list_[obj_list_.size() - 1].face_table[pending_mtl_name_].push_back(
        Vertex(vertices_[face_point.vertex_index],
            norms_[face_point.norm_index],
            texture_vertices_[face_point.texture_index]));

    face_point = face[l];
    obj_list_[obj_list_.size() - 1].face_table[pending_mtl_name_].push_back(
        Vertex(vertices_[face_point.vertex_index],
            norms_[face_point.norm_index],
            texture_vertices_[face_point.texture_index]));
  }
  return true;
}

bool
ObjParser::ParseObjLine(std::string *line)
{
  std::vector<std::string> tokens;
  Split(line, &tokens);

  if (tokens.size() == 0) return true;

  if (tokens[0] == "mtllib") return ParseMtlPath(&tokens);
  if (tokens[0] == "o") return ParseObjName(&tokens);
  if (tokens[0] == "v") return ParseVertex(&tokens);
  if (tokens[0] == "vt") return ParseTextureVertex(&tokens);
  if (tokens[0] == "vn") return ParseNorm(&tokens);
  if (tokens[0] == "usemtl") return ParseObjMtlName(&tokens);
  if (tokens[0] == "s") return ParseSmoothShading(&tokens);
  if (tokens[0] == "f") return ParseFace(&tokens);

  return true;
}

bool
ObjParser::ParseObj()
{
  Reader *reader;
  std::string line;

  if (obj_path_.find("http") == 0) {
    HttpReader *http_reader = new HttpReader();
    if (!http_reader->Download(obj_path_)) return false;
    reader = http_reader;
  } else {
    FileReader *file_reader = new FileReader();
    if (!file_reader->Open(obj_path_)) {
      std::cerr << "[error] File is not found: " << obj_path_ << std::endl;
      return false;
    }
    reader = file_reader;
  }

  vertices_.push_back(glm::vec3(0));
  texture_vertices_.push_back(glm::vec2(0));
  norms_.push_back(glm::vec3(0));  // TODO:

  while (reader->GetLine(&line)) {
    if (!ParseObjLine(&line)) {
      std::cerr << "[error] broken file: " << obj_path_ << std::endl;
      return false;
    }
  }

  // for (auto obj : obj_list_) {
  //   std::cout << "----------------" << std::endl;
  //   std::cout << "name: " << obj.name << std::endl;
  //   std::cout << "usemtl: " << obj.mtl_name << std::endl;
  //   std::cout << "s: " << obj.smooth_shading << std::endl;
  //   std::cout << "f: " << obj.faces.size() << std::endl;

  //   int triangle_count = 0;
  //   for (auto f : obj.faces) triangle_count += f.size() - 2;
  //   std::cout << "triangles: " << triangle_count << std::endl;
  // }
  // std::cout << "===================" << std::endl;
  // std::cout << "v: " << vertices_.size() << std::endl;
  // std::cout << "vt: " << textures_.size() << std::endl;
  // std::cout << "vn: " << norms_.size() << std::endl;

  delete reader;

  return true;
}

bool
ObjParser::ParseMtlName(std::vector<std::string> *tokens)
{
  if (tokens->size() < 2) return false;

  MtlObject mtl;
  mtl.name = tokens->at(1);
  mtl_list_.push_back(mtl);
  return true;
}

bool
ObjParser::ParseMtlNs(std::vector<std::string> *tokens)
{
  if (tokens->size() < 2) return false;
  if (mtl_list_.size() == 0) return false;

  mtl_list_[mtl_list_.size() - 1].shininess = std::stof(tokens->at(1));
  return true;
}

bool
ObjParser::ParseMtlKa(std::vector<std::string> *tokens)
{
  if (tokens->size() < 4) return false;
  if (mtl_list_.size() == 0) return false;

  glm::vec3 Ka(std::stof(tokens->at(1)), std::stof(tokens->at(2)),
      std::stof(tokens->at(3)));

  mtl_list_[mtl_list_.size() - 1].ambient = Ka;
  return true;
}

bool
ObjParser::ParseMtlKd(std::vector<std::string> *tokens)
{
  if (tokens->size() < 4) return false;
  if (mtl_list_.size() == 0) return false;

  glm::vec3 Kd(std::stof(tokens->at(1)), std::stof(tokens->at(2)),
      std::stof(tokens->at(3)));

  mtl_list_[mtl_list_.size() - 1].diffuse = Kd;
  return true;
}

bool
ObjParser::ParseMtlKs(std::vector<std::string> *tokens)
{
  if (tokens->size() < 4) return false;
  if (mtl_list_.size() == 0) return false;

  glm::vec3 Ks(std::stof(tokens->at(1)), std::stof(tokens->at(2)),
      std::stof(tokens->at(3)));

  mtl_list_[mtl_list_.size() - 1].specular = Ks;
  return true;
}

bool
ObjParser::ParseMtlKe(std::vector<std::string> *tokens)
{
  if (tokens->size() < 4) return false;
  if (mtl_list_.size() == 0) return false;

  glm::vec3 Ke(std::stof(tokens->at(1)), std::stof(tokens->at(2)),
      std::stof(tokens->at(3)));

  mtl_list_[mtl_list_.size() - 1].emissive = Ke;
  return true;
}

bool
ObjParser::ParseMtlNi(std::vector<std::string> *tokens)
{
  if (tokens->size() < 2) return false;
  if (mtl_list_.size() == 0) return false;

  mtl_list_[mtl_list_.size() - 1].optical_density = std::stof(tokens->at(1));
  return true;
}

bool
ObjParser::ParseMtlD(std::vector<std::string> *tokens)
{
  if (tokens->size() < 2) return false;
  if (mtl_list_.size() == 0) return false;

  mtl_list_[mtl_list_.size() - 1].opacity = std::stof(tokens->at(1));
  return true;
}

bool
ObjParser::ParseMtlIllum(std::vector<std::string> *tokens)
{
  if (tokens->size() < 2) return false;
  if (mtl_list_.size() == 0) return false;

  mtl_list_[mtl_list_.size() - 1].illum = std::stoi(tokens->at(1));
  return true;
}

bool
ObjParser::ParseMtlMapKd(std::vector<std::string> *tokens)
{
  if (tokens->size() < 2) return false;
  if (mtl_list_.size() == 0) return false;

  if (!CheckMtlTextureExist(&tokens->at(1))) return false;

  mtl_list_[mtl_list_.size() - 1].map_kd_path = tokens->at(1);
  return true;
}

bool
ObjParser::ParseMtlLine(std::string *line)
{
  std::vector<std::string> tokens;
  Split(line, &tokens);

  if (tokens.size() == 0) return true;

  if (tokens[0] == "newmtl") return ParseMtlName(&tokens);
  if (tokens[0] == "Ns") return ParseMtlNs(&tokens);
  if (tokens[0] == "Ka") return ParseMtlKa(&tokens);
  if (tokens[0] == "Kd") return ParseMtlKd(&tokens);
  if (tokens[0] == "Ks") return ParseMtlKs(&tokens);
  if (tokens[0] == "Ke") return ParseMtlKe(&tokens);
  if (tokens[0] == "Ni") return ParseMtlNi(&tokens);
  if (tokens[0] == "d") return ParseMtlD(&tokens);
  if (tokens[0] == "illum") return ParseMtlIllum(&tokens);
  if (tokens[0] == "map_Kd") return ParseMtlMapKd(&tokens);

  return true;
}

bool
ObjParser::CheckMtlTextureExist(std::string *path)
{
  if (*path == "") return true;
  if (textures_dir_path_ == "") return false;

  return std::binary_search(
      texture_file_names_.begin(), texture_file_names_.end(), *path);
}

bool
ObjParser::ReadMtlTextureFileNames()
{
  DIR *dir;
  struct dirent *diread;

  if (textures_dir_path_ == "") return true;

  if ((dir = opendir(textures_dir_path_.c_str())) == nullptr) {
    std::cerr << "Failed to open " << textures_dir_path_ << std::endl;
    return false;
  }

  while ((diread = readdir(dir)) != nullptr)
    if (diread->d_type == DT_REG) texture_file_names_.push_back(diread->d_name);
  closedir(dir);

  std::sort(texture_file_names_.begin(), texture_file_names_.end());

  return true;
}

bool
ObjParser::ParseMtl()
{
  Reader *reader;
  std::string line;

  if (mtl_path_ == "") return true;

  if (mtl_path_.find("http") == 0) {
    HttpReader *http_reader = new HttpReader();
    if (!http_reader->Download(mtl_path_)) return false;
    reader = http_reader;
  } else {
    FileReader *file_reader = new FileReader();
    if (!file_reader->Open(mtl_path_)) {
      std::cerr << "[error] File is not found" << mtl_path_ << std::endl;
      return false;
    }
    reader = file_reader;
  }

  if (target_mtl_path_ == "") return true;

  if (ExtractFileName(&mtl_path_) != ExtractFileName(&target_mtl_path_)) {
    std::cerr << "[error] mtl file name is not same: " << mtl_path_
              << std::endl;
    return false;
  }

  if (!ReadMtlTextureFileNames()) return false;

  while (reader->GetLine(&line)) {
    if (!ParseMtlLine(&line)) {
      std::cerr << "[error] broken file: " << mtl_path_ << std::endl;
      return false;
    }
  }

  for (auto mtl : mtl_list_) {
    mtl_table_[mtl.name] = mtl;
  }

  // for (auto mtl : mtl_list_) {
  //   std::cout << "----------------" << std::endl;
  //   std::cout << "name: " << mtl.name << std::endl;
  //   std::cout << "Ns: " << mtl.shininess << std::endl;
  //   std::cout << "Ka: " << mtl.ambient[0] << " " << std::endl;
  //   std::cout << "Kd: " << mtl.diffuse[0] << " " << std::endl;
  //   std::cout << "Ks: " << mtl.specular[0] << " " << std::endl;
  //   std::cout << "Ke: " << mtl.emissive[0] << " " << std::endl;
  //   std::cout << "Ni: " << mtl.optical_density << std::endl;
  //   std::cout << "d: " << mtl.opacity << std::endl;
  //   std::cout << "illum: " << mtl.illum << std::endl;
  //   std::cout << "map_Kd: " << mtl.map_kd_path << std::endl;
  // }

  delete reader;

  return true;
}

bool
ObjParser::Parse()
{
  if (!ParseObj()) {
    std::cerr << "[error] failed to parse obj file: " << obj_path_ << std::endl;
    return false;
  }
  if (!ParseMtl()) {
    std::cerr << "[error] failed to parse mtl file: " << mtl_path_ << std::endl;
    return false;
  }

  return true;
}
