#pragma once
#include "object.h"
#include "token.h"
#include <unordered_map>

class Environment {
private:
public:
  std::unordered_map<std::string, Object> env_map;
  Environment *enclosing = nullptr;

public:
  Environment() = default;
  Environment(Environment *enclosing) : enclosing(enclosing){};
  void define(const std::string &name, const Object &value);
  [[nodiscard]] bool assign(const std::string &name, const Object &value);
  Object *get(const Token &t);
  Object *get(const std::string &name);
  Object *get_at(int distance, const std::string &name);
  bool assign_at(int distance, const std::string &name, const Object &value);
  std::string print();
  ~Environment();
};
