#pragma once

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <istream>
#include <numeric>
#include <vector>

struct Token {
  enum Type {
    NONE,

    NUMBER,
    CHARACTER,
    STRING,

    // Reserved identifiers
    EOL,
    END,
    REGISTER,
    DATA,
    TAG_DECL,
    IDENTIFIER,

    // Other
    ERROR

  };

  std::wstring wstr() const;
  std::string as_str() const;
  std::string fmt() const;

  unsigned as_number() const;
  char as_char() const;
  wchar_t as_wchar() const;

  Type type;
  std::vector<std::byte> data = {};
};

namespace {

struct TokenParser {
  int len = 0;

  unsigned num_base = 0;

  unsigned value = 0;

  char prev_char;
  std::string identifier = {};

  Token::Type type = Token::NONE;

  Token ingest(char ch);

 private:
  bool first_byte(char ch);

  bool consume_IDENTIFIER(char ch);
  bool consume_NUMBER(char ch);
  bool consume_CHARACTER(char ch);
  bool consume_STRING(char ch);

  Token produce();
};

}  // namespace

std::vector<Token> tokenize(std::istream& is);
