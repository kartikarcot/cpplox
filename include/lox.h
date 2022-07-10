#pragma once
#include <string>
#include "scanner.h"
#include "parser.h"

class Lox {
  bool had_error_ = false;
	Scanner scanner;
	Parser parser;
	public:
  void error(const std::string &message, int line_no);
  void report(const std::string &message, const std::string &where, int line_no);
  void run(const std::string &lox_code);
  void run_prompt();
  void run_file(const std::string &file_path);
};
