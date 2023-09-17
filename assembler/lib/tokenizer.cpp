#include "tokenizer.hpp"

#include <cassert>
#include <format>
#include <ios>
#include <iostream>
#include <optional>
#include <stdexcept>

std::vector<std::byte> str_to_bytes(std::string str) {
  std::vector<std::byte> out;
  out.reserve(2 * str.size());
  for (auto c : str) {
    out.push_back(std::byte(c));
    out.push_back(std::byte(0));
  }

  return out;
}

constexpr std::optional<char> escape(char ch) noexcept {
  switch (ch) {
    case '0':
      return 0;
    case 'n':
      return '\n';
    case 't':
      return '\t';
    case 'v':
      return '\v';
    case 'b':
      return '\b';
    case 'r':
      return '\r';
    case 'f':
      return '\f';
    case 'a':
      return '\a';
    case '\\':
      return '\\';
    case '\'':
      return '\'';
    case '"':
      return '\"';
  }

  return {};
}

Token TokenParser::ingest(char ch) {
  const bool done = [this, ch]() -> bool {
    switch (type) {
      case Token::EOL:
        // Fallthrough
      case Token::NONE:
        return first_byte(ch);
      case Token::IDENTIFIER:
        return consume_IDENTIFIER(ch);
      case Token::NUMBER:
        return consume_NUMBER(ch);
      case Token::CHARACTER:
        return consume_CHARACTER(ch);
      case Token::STRING:
        return consume_STRING(ch);
      case Token::REGISTER:
      case Token::DATA:
      case Token::END:
      case Token::TAG_DECL:
        assert(0);  // Unreachable
      case Token::ERROR:
        throw std::runtime_error("Consumed character after having errored out");
    }
    assert(0);  // Unreachable
    return true;
  }();

  ++len;
  Token r{};
  if (done) {
    r = produce();
    *this = TokenParser{};
  }

  return r;
}

bool TokenParser::consume_IDENTIFIER(char ch) {
  // Detects keywords and tags
  if (ch == ' ' || ch == '\n') {
    return true;
  }
  identifier.push_back(ch);
  return false;
}

bool TokenParser::consume_NUMBER(char ch) {
  if (num_base == 0 && len == 1) {
    if (ch == 'b') {
      num_base = 0b10;
      len = -1;
      return false;
    }
    if (ch == 'x') {
      num_base = 0x10;
      len = -1;
      return false;
    }
    if (std::isdigit(ch)) {
      num_base = 010;
      value = static_cast<unsigned>(ch - '0');
      len = -1;
      return false;
    }
  }

  if (ch == ' ' || ch == '\n') {
    if (len != 0) {
      return true;
    }
    type = Token::ERROR;
    identifier = std::format(
        "Unexpected end of digit in base-{} number literal", num_base);
    return true;
  }

  unsigned digit = 0;
  if (ch == '_') {
    // Ignored separator
    return false;
  } else if ('0' <= ch && ch <= '9') {
    digit = static_cast<unsigned>(ch - '0');
  } else if ('a' <= ch && ch <= 'z') {
    digit = static_cast<unsigned>(ch - 'a');
  } else if ('A' <= ch && ch <= 'Z') {
    digit = static_cast<unsigned>(ch - 'A');
  } else {
    type = Token::ERROR;
    identifier = std::format("Unexpected digit in base-{} number literal: {}",
                             num_base, ch);
    return true;
  }

  if (digit > num_base) {
    type = Token::ERROR;
    identifier = std::format("Unexpected digit in base-{} number literal: {}",
                             num_base, ch);
    return true;
  }

  value = value * num_base + digit;
  return false;
}

bool TokenParser::consume_CHARACTER(char ch) {
  switch (len) {
    case 0:
      assert(false);  // Unreachable
      return false;
    case 1:
      if (ch == '\n') {
        type = Token::ERROR;
        identifier = "Unexpected end of character literal";
      }
      if (ch == '\\') {
        prev_char = ch;
        return false;
      }
      prev_char = 0;
      value = static_cast<unsigned>(ch);
      return false;
    case 2:
      if (ch == '\n') {
        type = Token::ERROR;
        identifier = "Unexpected end of character literal";
      }

      if (prev_char == '\\') {
        --len;
        const auto x = escape(ch);
        if (x.has_value()) {
          prev_char = 0;
          value = static_cast<unsigned>(ch);
          return false;
        }
        type = Token::ERROR;
        identifier = std::format("Invalid escaped character ascii {}", int(ch));
        return true;
      }

      if (ch != '\'') {
        type = Token::ERROR;
        identifier = std::format(
            "Expected closing single quote ('), got ascii {}", int(ch));
        return true;
      }
      return false;
    default:
      if (ch == ' ' || ch == '\n') {
        return true;
      }
      type = Token::ERROR;
      identifier = std::format(
          "Unexpected character after closing quote ('), got ascii {}",
          int(ch));
      return true;
  }
}

bool TokenParser::consume_STRING(char ch) {
  if (len < 0 && (ch == ' ' || ch == '\n')) {
    return true;
  }

  if (len < 0) {
    type = Token::ERROR;
    identifier = std::format(
        "Unexpected character after closing quote (\"), got: ascii {}",
        int(ch));
  }

  if (ch == '\n') {
    type = Token::ERROR;
    identifier = "Missing endquote to close string literal";
    return true;
  }

  // Escaped characters
  if (prev_char == '\\') {
    const auto x = escape(ch);
    if (x.has_value()) {
      prev_char = *x;
      return false;
    }
    type = Token::ERROR;
    identifier = std::format("Invalid escaped character \\{}", ch);
    return true;
  }

  // End of string
  if (ch == '"') {
    identifier.push_back(prev_char);
    identifier.push_back('\0');
    len = -0xff;
    return false;
  }

  // Regular characters
  identifier.push_back(prev_char);
  prev_char = ch;
  return false;
}

bool TokenParser::first_byte(char ch) {
  if (ch == '\n') {
    type = Token::EOL;
    return true;
  }

  if (ch == ' ') {
    return false;
  }

  if (ch == '0') {
    type = Token::NUMBER;
    return false;
  }

  if (ch > '1' && ch <= '9') {
    num_base = 10;
    type = Token::NUMBER;
    value = static_cast<unsigned>(ch - '0');
    return false;
  }

  if (ch == '\'') {
    type = Token::CHARACTER;
    return false;
  }

  if (ch == '"') {
    type = Token::STRING;
  }

  type = Token::IDENTIFIER;
  identifier.push_back(ch);
  return false;
}

Token TokenParser::produce() {
  auto r = [this]() -> Token {
    switch (type) {
      case Token::NUMBER:
        assert(value <= 0xffff);
        return Token{
            .type = Token::NUMBER,
            .data = {std::byte(value & 0xff), std::byte(value & 0xff00)}};
      case Token::CHARACTER:
        return Token{
            .type = Token::CHARACTER,
            .data = {std::byte(value & 0xff), std::byte(value & 0xff00)}};
      case Token::STRING: {
        return Token{
            .type = Token::STRING,
            .data = str_to_bytes(identifier),
        };
      }
      case Token::EOL:
        return Token{.type = Token::EOL};
      case Token::IDENTIFIER:
        if (identifier == "data") {
          return Token{Token::DATA, {}};
        }
        if (identifier.starts_with('r') && identifier.size() == 2 &&
            std::isdigit(identifier[1])) {
          return Token{
              .type = Token::REGISTER,
              .data = {std::byte(identifier[1] - '0'), std::byte(0x80)}};
        }

        if (identifier.ends_with(":")) {
          identifier.pop_back();
          return Token{.type = Token::TAG_DECL,
                       .data = str_to_bytes(identifier)};
        }

        return Token{.type = Token::IDENTIFIER,
                     .data = str_to_bytes(identifier)};

      case Token::ERROR:
        return Token{.type = Token::ERROR, .data = str_to_bytes(identifier)};
      case Token::NONE:
        return Token{.type = Token::NONE};
      case Token::REGISTER:
      case Token::TAG_DECL:
      case Token::END:
      case Token::DATA:
        assert(false);  // Unreachable code
        return Token{.type = Token::ERROR};
    }

    assert(false);  // Unreachable code
    return Token{.type = Token::ERROR};
  }();

  *this = TokenParser{};
  return r;
}

std::vector<Token> tokenize(std::istream &is) {
  std::vector<Token> tokenized;
  TokenParser p;

  std::size_t row = 1;
  std::size_t col = 0;

  const auto next_char = [&](char ch) -> bool {
    auto token = p.ingest(ch);
    if (token.type == Token::NONE) {
      return false;
    }

    tokenized.push_back(token);

    if (token.type == Token::ERROR) {
      std::cerr << std::format("Error parsing line {}, col {}: {}\n", row, col,
                               token.as_str());
      return true;
    }

    if (ch == '\n') {
      tokenized.push_back(Token{.type = Token::EOL});
      ++row;
      col = 0;
    }

    return false;
  };

  auto skipline = false;
  while (is) {
    const auto ch = static_cast<char>(is.get());
    ++col;

    if (ch == EOF) {
      next_char('\n');
      break;
    }

    if (ch == '\n') {
      skipline = false;
    }

    if (skipline) {
      continue;
    }

    skipline = next_char(ch);
  }

  tokenized.push_back(Token{.type = Token::END});
  return tokenized;
}

std::wstring Token::wstr() const {
  std::wstring out;
  out.reserve(out.size() / 2 + 1);
  std::size_t i;
  for (i = 1; i < data.size(); i += 2) {
    wchar_t ch = wchar_t(data[i - 1]) | wchar_t(data[i]) << 8;
    out.push_back(ch);
  }

  if (data.size() % 2 == 1) {
    wchar_t ch = wchar_t(data[data.size() - 1]);
    out.push_back(ch);
  }

  return out;
}

std::string Token::as_str() const {
  std::string out;
  out.reserve(data.size() / 2 + 1);
  for (std::size_t i = 0; i < data.size(); i += 2) {
    out.push_back(static_cast<char>(data[i]));
  }

  return out;
}

unsigned Token::as_number() const {
  unsigned n = 0;

  if (data.size() != 0) {
    n = static_cast<unsigned>(data[0]);
  }
  if (data.size() != 1) {
    n |= static_cast<unsigned>(data[1]) << 8;
  }

  return n;
}

char Token::as_char() const { return static_cast<char>(as_number()); }

wchar_t Token::as_wchar() const { return static_cast<wchar_t>(as_number()); }

std::string Token::fmt() const {
  switch (type) {
    case Token::NONE:
      return "{NONE}";
    case Token::NUMBER:
      return std::format("<NUMBER {}>", as_number());
    case Token::CHARACTER:
      return std::format("<CHARACTER {}>", as_char());
    case Token::STRING:
      return std::format("<STRING {}>", as_str());
    case Token::EOL:
      return "<EOL>";
    case Token::END:
      return "<END>";
    case Token::REGISTER:
      return std::format("<REGISTER {}>", data.empty() ? -1 : int(data[0]));
    case Token::DATA:
      return "<DATA>";
    case Token::TAG_DECL:
      return std::format("<TAG_DECL {}>", as_str());
    case Token::IDENTIFIER:
      return std::format("<IDENTIFIER {}>", as_str());
    case Token::ERROR:
      return std::format("<ERROR {}>", as_str());
  }

  assert(false);  // Unreachable code
  return "<ERROR BAD CODE!>";
}