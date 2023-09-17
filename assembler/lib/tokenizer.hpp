#pragma once

#include <vector>
#include <iosfwd>
#include <string>

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
  Token::Type type = Token::NONE;

  // Numeric helpers
  unsigned num_base = 0;
  unsigned value = 0;

  // Text helpers helpers
  char prev_char;
  std::string identifier = {};

  Token consume(char ch);

 private:
  bool error(char ch, std::string&& msg) {
    prev_char = ch;
    type = Token::ERROR;
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
};

}  // namespace

std::vector<Token> tokenize(std::istream& is);

std::ostream& fmt_tokens(std::ostream& out, std::vector<Token> const& tokenized);