#pragma once

#include "arch/arch.hpp"
#include "grammar.hpp"

#include <vector>
#include <iosfwd>
#include <string>

namespace {

struct TokenParser {
  std::string_view file_name;
  unsigned row = 1;
  unsigned col = 1;
  
  unsigned start_row = 1;
  unsigned start_col = 1;
  
  int len = 0;
  Symbol type = Symbol::NONE;

  // Numeric helpers
  unsigned num_base = 0;
  unsigned value = 0;

  // Text helpers
  char prev_char = {};
  std::string identifier = {};

  Token consume(char ch);

 private:
  bool error(char ch, std::string&& msg) {
    prev_char = ch;
    type = Symbol::ERROR;
    identifier = std::forward<std::string>(msg);
    return false;
  }

  void first_byte(char ch);

  bool consume_EOL(char ch);
  bool consume_IDENTIFIER(char ch);
  bool consume_NUMBER(char ch);
  bool consume_CHARACTER(char ch);
  bool consume_STRING(char ch);
  bool consume_ERROR(char ch);

  Token finalize();

  Token finalize_NUMBER();
  Token finalize_CHARACTER();
  Token finalize_STRING();
  Token finalize_IDENTIFIER();

  void clear();
};

}  // namespace

std::pair<std::vector<Token>, bool> tokenize(std::string file_name);

std::ostream& fmt_tokens(std::ostream& out, std::vector<Token> const& tokenized);