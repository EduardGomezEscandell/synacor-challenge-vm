#include "tokenizer.hpp"

#include <cassert>
#include <cctype>
#include <format>
#include <ios>
#include <iostream>
#include <optional>
#include <stdexcept>

#include "arch/arch.hpp"

std::vector<std::byte> str_to_bytes(std::string str);
constexpr std::optional<char> escape(char ch);

std::vector<Token> tokenize(std::istream& is) {
  std::vector<Token> tokenized;
  TokenParser p;

  std::size_t row = 1;
  std::size_t col = 0;

  const auto consume = [&](char ch) {
    auto token = p.consume(ch);
    if (token.type == Token::NONE) {
      return;
    }

    tokenized.push_back(token);

    if (token.type == Token::ERROR) {
      std::cerr << std::format("Error parsing line {}, col {}: {}\n", row, col,
                               token.as_str());
    }
  };

  while (is) {
    const auto ch = static_cast<char>(is.get());
    ++col;

    if (ch == EOF) {
      consume('\n');
      break;
    }

    consume(ch);

    if (ch == '\n') {
      ++row;
      col = 0;
    }
  }

  consume(EOF);
  consume(0);

  return tokenized;
}

Token TokenParser::consume(char ch) {
  const bool done = [this, ch]() -> bool {
    switch (type) {
      case Token::NONE:
        len = 0;
        first_byte(ch);
        return false;
      case Token::EOL:
        return consume_EOL(ch);
      case Token::UNKNOWN_IDENTIFIER:
        return consume_IDENTIFIER(ch);
      case Token::NUMBER_LITERAL:
        return consume_NUMBER(ch);
      case Token::CHARACTER_LITERAL:
        return consume_CHARACTER(ch);
      case Token::STRING_LITERAL:
        return consume_STRING(ch);
      case Token::ERROR:
        return consume_ERROR(ch);
      case Token::END:
        return true;
        // The following types are not possible until finalize() is called.
      case Token::VERB:
      case Token::REGISTER:
      case Token::TAG_DECL:
      case Token::TAG_REF:
        assert(0);  // Unreachable
    }
    assert(0);  // Unreachable
    return false;
  }();

  Token r{};
  if (done) {
    r = finalize();
    first_byte(ch);
  }

  ++len;

  return r;
}

void TokenParser::first_byte(char ch) {
  if (ch == EOF) {
    type = Token::END;
    return;
  }

  if (ch == '\n') {
    type = Token::EOL;
    return;
  }

  if (ch == ' ') {
    return;
  }

  if (ch == '0') {
    type = Token::NUMBER_LITERAL;
    return;
  }

  if ('1' <= ch && ch <= '9') {
    num_base = 10;
    type = Token::NUMBER_LITERAL;
    value = static_cast<unsigned>(ch - '0');
    return;
  }

  if (ch == '\'') {
    --len;
    type = Token::CHARACTER_LITERAL;
    return;
  }

  if (ch == '"') {
    type = Token::STRING_LITERAL;
    --len;
    return;
  }

  type = Token::UNKNOWN_IDENTIFIER;
  identifier.push_back(ch);
}

bool TokenParser::consume_EOL(char ch) {
  if (ch == '\n' || ch == ' ') {
    return false;
  }
  return true;
}

bool TokenParser::consume_IDENTIFIER(char ch) {
  // Detects keywords and tags
  if (ch == ' ' || ch == '\n') {
    return true;
  }

  if (std::isalnum(ch)) {
    identifier.push_back(ch);
    return false;
  }

  switch (ch) {
    case '-':
    case '_':
    case ':':
    case '.':
      identifier.push_back(ch);
      return false;
  }

  return error(ch,
               std::format("Unexpected character in identifier: ascii {}", ch));
}

bool TokenParser::consume_NUMBER(char ch) {
  if (num_base == 0 && len == 1) {
    if (ch == 'b') {
      num_base = 0b10;
      len -= 2;  // the prefix
      return false;
    }
    if (ch == 'x') {
      num_base = 0x10;
      len -= 2;  // the prefix
      return false;
    }
    if (std::isdigit(ch)) {
      num_base = 010;
      value = static_cast<unsigned>(ch - '0');
      if (value > num_base) {
        return error(
            ch, std::format("Unexpected digit in base-{} number literal: {}",
                            num_base, ch));
      }
      len -= 1;  // the prefix
      return false;
    }

    if (ch == ' ' || ch == '\n') {
      value = 0;
      return true;
    }

    return error(ch, std::format("Unknown prefix"));
  }

  if (ch == ' ' || ch == '\n') {
    if (len != 0) {
      return true;
    }
    return error(
        ch, std::format("Unexpected end of digit in base-{} number literal",
                        num_base));
  }

  unsigned digit = 0;
  if (ch == '_') {
    // Ignored separator
    --len;
    return false;
  } else if ('0' <= ch && ch <= '9') {
    digit = static_cast<unsigned>(ch - '0');
  } else if ('a' <= ch && ch <= 'z') {
    digit = static_cast<unsigned>(ch - 'a');
  } else if ('A' <= ch && ch <= 'Z') {
    digit = static_cast<unsigned>(ch - 'A');
  } else {
    return error(ch,
                 std::format("Unexpected digit in base-{} number literal: {}",
                             num_base, ch));
  }

  if (digit > num_base) {
    return error(ch,
                 std::format("Unexpected digit in base-{} number literal: {}",
                             num_base, ch));
  }

  value = value * num_base + digit;
  return false;
}

bool TokenParser::consume_CHARACTER(char ch) {
  switch (len) {
    case 0:
      if (ch == '\n') {
        return error(ch, "Unexpected end of character literal");
      }
      if (ch == '\\') {
        prev_char = ch;
        return false;
      }
      prev_char = 0;
      value = static_cast<unsigned>(ch);
      return false;
    case 1:
      if (ch == '\n') {
        return error(ch, "Unexpected end of character literal");
      }

      if (prev_char == '\\') {
        --len;
        const auto x = escape(ch);
        if (x.has_value()) {
          prev_char = 0;
          value = static_cast<unsigned>(ch);
          return false;
        }
        return error(
            ch, std::format("Invalid escaped character ascii {}", int(ch)));
      }

      if (ch != '\'') {
        return error(
            ch, std::format("Expected closing single quote ('), got ascii {}",
                            int(ch)));
      }
      return false;
    default:
      if (ch == ' ' || ch == '\n') {
        return true;
      }
      return error(
          ch, std::format(
                  "Unexpected character after closing quote ('), got ascii {}",
                  int(ch)));
  }
}

bool TokenParser::consume_STRING(char ch) {
  if (len < 0 && (ch == ' ' || ch == '\n')) {
    return true;
  }

  if (len < 0) {
    return error(
        ch, std::format(
                "Unexpected character after closing quote (\"), got: ascii {}",
                int(ch)));
  }

  if (ch == '\n') {
    return error(ch, std::format("Missing endquote to close string literal"));
  }

  // Escaped characters
  if (prev_char == '\\') {
    const auto x = escape(ch);
    if (x.has_value()) {
      prev_char = *x;
      return false;
    }
    return error(ch, std::format("Invalid escaped character \\{}", ch));
  }

  // End of string
  if (ch == '"') {
    if (len != 0) {
      identifier.push_back(prev_char);
    }
    len = -0xff;
    return false;
  }

  // Regular characters
  if (len != 0) {
    identifier.push_back(prev_char);
  }
  prev_char = ch;
  return false;
}

bool TokenParser::consume_ERROR(char ch) {
  // Consume the entire line
  if (prev_char == '\n') {
    return true;
  }
  prev_char = ch;

  return false;
}

Token TokenParser::finalize() {
  auto r = [this]() -> Token {
    switch (type) {
      case Token::NUMBER_LITERAL:
        return finalize_NUMBER();
      case Token::CHARACTER_LITERAL:
        return finalize_CHARACTER();
      case Token::STRING_LITERAL:
        return finalize_STRING();
      case Token::UNKNOWN_IDENTIFIER:
        return finalize_IDENTIFIER();
      case Token::ERROR:
        return {.type = Token::ERROR, .data = str_to_bytes(identifier)};
      case Token::EOL:
        return {.type = Token::EOL};
      case Token::NONE:
        return {.type = Token::NONE};
      case Token::END:
        return {.type = Token::END};
      case Token::REGISTER:
      case Token::TAG_DECL:
      case Token::TAG_REF:
      case Token::VERB:
        assert(false);  // Unreachable code
        return {.type = Token::ERROR};
    }

    assert(false);  // Unreachable code
    return {.type = Token::ERROR};
  }();

  *this = TokenParser{};
  return r;
}

Token TokenParser::finalize_NUMBER() {
  assert(value <= 0xffff);
  return {.type = Token::NUMBER_LITERAL,
          .data = {std::byte(value & 0xff), std::byte(value & 0xff00)}};
}

Token TokenParser::finalize_CHARACTER() {
  return {.type = Token::CHARACTER_LITERAL,
          .data = {std::byte(value & 0xff), std::byte(value & 0xff00)}};
}

Token TokenParser::finalize_STRING() {
  return {
      .type = Token::STRING_LITERAL,
      .data = str_to_bytes(identifier),
  };
}

Token TokenParser::finalize_IDENTIFIER() {
  if (const auto id = from_string(identifier); id != Verb::ERROR) {
    return {
        .type = Token::VERB,
        .data = {std::byte(id & 0xff), std::byte{}},
    };
  }

  if (identifier.starts_with('r') && identifier.size() == 2 &&
      std::isdigit(identifier[1])) {
    return {.type = Token::REGISTER,
            .data = {std::byte(identifier[1] - '0'), std::byte(0x80)}};
  }

  if (identifier.ends_with(":")) {
    identifier.pop_back();
    return {.type = Token::TAG_DECL, .data = str_to_bytes(identifier)};
  }

  return {.type = Token::TAG_REF, .data = str_to_bytes(identifier)};
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
    case Token::NUMBER_LITERAL:
      return std::format("<NUMBER {}>", as_number());
    case Token::CHARACTER_LITERAL:
      return std::format("<CHARACTER {}>", as_char());
    case Token::STRING_LITERAL:
      return std::format("<STRING {}>", as_str());
    case Token::EOL:
      return "<EOL>";
    case Token::END:
      return "<END>";
    case Token::REGISTER:
      return std::format("<REGISTER {}>", data.empty() ? -1 : int(data[0]));
    case Token::TAG_DECL:
      return std::format("<TAG_DECL {}>", as_str());
    case Token::TAG_REF:
      return std::format("<TAG_REF {}>", as_str());
    case Token::VERB:
      return std::format("<VERB {}>", to_string(static_cast<Verb>(as_number())));
    case Token::UNKNOWN_IDENTIFIER:
      return std::format("<UNKNOWN_IDENTIFIER {}>", as_str());
    case Token::ERROR:
      return std::format("<ERROR {}>", as_str());
  }

  assert(false);  // Unreachable code
  return "<ERROR BAD CODE!>";
}

std::vector<std::byte> str_to_bytes(std::string str) {
  std::vector<std::byte> out;
  out.reserve(2 * str.size());
  for (auto c : str) {
    out.push_back(std::byte(c));
    out.push_back(std::byte(0));
  }

  return out;
}

constexpr std::optional<char> escape(char ch) {
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

std::ostream& fmt_tokens(std::ostream& out,
                         std::vector<Token> const& tokenized) {
  bool firstchar = true;
  for (auto const& token : tokenized) {
    if (!firstchar) {
      out << ' ';
    }
    firstchar = false;
    out << token.fmt();
    if (token.type == Token::EOL || token.type == Token::ERROR) {
      out << '\n';
      firstchar = true;
    }
  }
  out << '\n';
  return out;
}