#include "logger.h"
#include "stringbuffer.h"
#include "utils.h"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

void generate_base_ast(std::string &ast_code, const std::string &class_name) {
  StringBuffer buffer;
  buffer.write_line("class %s{", class_name.c_str());
  buffer.increase_indent();
  buffer.write_line("public:");
  buffer.increase_indent();
  buffer.write_line("size_t line_no = 0;");
  buffer.write_line("template <typename T, typename V>");
  buffer.write_line("T accept(V v) const {");
  buffer.increase_indent();
  buffer.write_line("return v->visit(this);");
  buffer.decrease_indent();
  buffer.write_line("}");
  buffer.write_line("virtual std::string print_type() {");
  buffer.increase_indent();
  buffer.write_line("return \"Expr\";");
  buffer.decrease_indent();
  buffer.write_line("}");
  buffer.write_line("virtual ~%s() {} ", class_name.c_str());
  buffer.decrease_indent();
  buffer.decrease_indent();
  buffer.decrease_indent();
  buffer.write_line("};");
  buffer.write_line("%s", "");
  buffer.get_contents(ast_code);
}

void generate_ast(const std::string &rule, const std::string &base_name,
                  std::string &ast_code) {
  // get the class name
  auto rule_split = split(rule, "=>");
  assert(rule_split.size() == 2);
  for (auto &item : rule_split) {
    trim(item);
  }
  std::string class_name = rule_split[0];
  rule_split = split(rule_split[1], ",");
  assert(!rule_split.empty());
  std::vector<std::pair<std::string, std::string>> components;
  for (auto &item : rule_split) {
    trim(item);
    auto item_split = split(item, " ");
    assert(item_split.size() == 2);
    auto &type = item_split[0];
    trim(type);
    assert(!type.empty());
    auto &name = item_split[1];
    trim(name);
    assert(!name.empty());
    components.push_back({type, name});
  }

  StringBuffer buffer;
  buffer.write_line("class %s : public %s {", class_name.c_str(),
                    base_name.c_str());
  buffer.increase_indent();
  buffer.write_line("public:");
  buffer.increase_indent();
  for (const auto &item : components) {
    buffer.write_line("%s %s;", item.first.c_str(), item.second.c_str());
  }
  buffer.write_line("virtual std::string print_type() {");
  buffer.increase_indent();
  buffer.write_line("return \"%s\";", class_name.c_str());
  buffer.decrease_indent();
  buffer.write_line("}");
  buffer.write_line("");
  buffer.write_line("~%s() {", class_name.c_str());
  buffer.increase_indent();
  for (const auto &item : components) {
    if (*(item.first.rbegin()) == '*') {
      buffer.write_line("delete %s;", item.second.c_str());
    }
  }
  buffer.decrease_indent();
  buffer.write_line("}");
  buffer.decrease_indent();
  buffer.decrease_indent();
  buffer.decrease_indent();
  buffer.write_line("};");
  buffer.write_line("%s", "");
  buffer.get_contents(ast_code);
}

void generate_preamble(std::string &code) {
  StringBuffer buffer;
  buffer.write_line("#pragma once");
  buffer.write_line("#include \"utils.h\"");
  buffer.write_line("#include \"token.h\"");
  buffer.write_line("%s", "");
  buffer.write_line("%s", "");
  buffer.get_contents(code);
  return;
}

bool write_code(std::ofstream &ofs, const std::string &ast_code) {
  ofs.write(ast_code.c_str(), ast_code.size());
  return true;
}

bool generate_code(std::ofstream &ofs, const std::string file_contents) {
  std::stringstream input_stringstream(file_contents);
  std::string rule;
  std::string code;
  generate_preamble(code);
  if (!write_code(ofs, code)) {
    CLog::FLog(LogLevel::ERROR, LogCategory::ALL,
               "Could not write contents to output file");
    return false;
  }
  while (true) {
    if (!getline(input_stringstream, rule)) {
      break;
    }
    if (rule.find("Basename") == std::string::npos) {
      CLog::FLog(LogLevel::ERROR, LogCategory::ALL,
                 "Could not find the Basename");
      return false;
    }
    auto basename_split = split(rule, " ");
    assert(basename_split.size() == 2);
    std::string basename = basename_split[1];
    trim(basename);
    generate_base_ast(code, basename);
    if (!write_code(ofs, code)) {
      CLog::FLog(LogLevel::ERROR, LogCategory::ALL,
                 "Could not write contents to output file");
      return false;
    }
    while (getline(input_stringstream, rule)) {
      if (rule == "---") {
        break;
      }
      generate_ast(rule, basename, code);
      if (!write_code(ofs, code)) {
        CLog::FLog(LogLevel::ERROR, LogCategory::ALL,
                   "Could not write contents to output file");
        return false;
      }
    }
  }
  return true;
}

int main(int argc, char **argv) {
  if (argc != 3) {
    CLog::FLog(LogLevel::INFO, LogCategory::ALL,
               "The correct usage is ast_generator grammer_file_path "
               "output_file_path");
    return -1;
  }
  std::string file_path = std::string(argv[1]);
  if (!fs::exists(file_path)) {
    CLog::FLog(LogLevel::ERROR, LogCategory::ALL, "No file called %s exists",
               file_path.c_str());
    return -1;
  }
  std::string output_file_path = std::string(argv[2]);
  std::ofstream ofs(output_file_path, std::ofstream::out);
  if (!ofs.is_open()) {
    CLog::FLog(LogLevel::ERROR, LogCategory::ALL,
               "Output file could not be created or opened: %s",
               output_file_path.c_str());
    return -1;
  }
  std::string file_contents = read_file_into_string(std::string(argv[1]));
  if (!generate_code(ofs, file_contents))
    return -1;
  ofs.close();
  return 0;
}
