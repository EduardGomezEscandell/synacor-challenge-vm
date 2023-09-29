#include "tokenizer.hpp"

#include <cassert>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <iostream>
#include <optional>
#include <stdexcept>

#include "arch/arch.hpp"
#include "assembler/lib/grammar.hpp"

std::vector<std::byte> str_to_bytes(std::string str);
constexpr std::optional<char> escape(char ch);

std::pair<std::vector<Token>, bool> tokenize(std::string file_name) {
  if(!std::filesystem::exists(file_name)) {
    std::cerr << std::format("File {} does not exist\n", file_name);
    return {{}, false};
  }

  std::ifstream f(file_name.c_str(), std::ios::binary);
  if(!f.good()) {
    std::cerr << std::format("Could not open file {}\n", file_name);
    return {{}, false};
  }

  std::vector<Token> tokenized;
  TokenParser p{.file_name = std::move(file_name)};

  const auto consume = [&](char ch) -> bool {
    auto token = p.consume(ch);
    if (token.symbol == Symbol::NONE) {
      return true;
    }

    tokenized.push_back(token);

    if (token.symbol == Symbol::ERROR) {
      std::cerr << std::format("{}: tokenization error: {}\n", token.location(),
                               token.as_str());
      return false;
    }

    return true;
  };

  unsigned errcount = 0;
  while (f) {
    const auto ch = static_cast<char>(f.get());

    if (ch == EOF) {
      errcount += consume('\n') ? 0 : 1;
      break;
    }

    errcount += consume(ch) ? 0 : 1;
  }

  errcount += consume(EOF) ? 0 : 1;
  errcount += consume(0) ? 0 : 1;

  return {std::move(tokenized), errcount == 0};
}

Token TokenParser::consume(char ch) {
  const bool done = [this, ch]() -> bool {
    switch (type) {
      case Symbol::NONE:
        len = 0;
        first_byte(ch);
        return false;
      case Symbol::END:
        return true;
      case Symbol::EOL:
        return consume_EOL(ch);
      case Symbol::UNKNOWN_IDENTIFIER:
        return consume_IDENTIFIER(ch);
      case Symbol::NUMBER_LITERAL:
        return consume_NUMBER(ch);
      case Symbol::CHARACTER_LITERAL:
        return consume_CHARACTER(ch);
      case Symbol::STRING_LITERAL:
        return consume_STRING(ch);
      case Symbol::ERROR:
        return consume_ERROR(ch);
        // The following types are not possible until finalize() is called.
      case Symbol::VERB:
      case Symbol::REGISTER:
      case Symbol::TAG_DECL:
      case Symbol::TAG_REF:
      CASE_NONTERMINAL:
        assert(0);  // Unreachable
    }
    assert(0);  // Unreachable
    return false;
  }();

  Token r{};
  if (done) {
    r = finalize();
    clear();
    first_byte(ch);
  }

  ++len;
  if (ch == '\n') {
    ++row;
    col = 1;
  } else {
    ++col;
  }

  return r;
}

void TokenParser::first_byte(char ch) {
  if (ch == EOF) {
    type = Symbol::END;
    return;
  }

  if (ch == '\n') {
    type = Symbol::EOL;
    return;
  }

  if (ch == ' ') {
    ++start_col;
    return;
  }

  if (ch == '0') {
    type = Symbol::NUMBER_LITERAL;
    return;
  }

  if ('1' <= ch && ch <= '9') {
    num_base = 10;
    type = Symbol::NUMBER_LITERAL;
    value = static_cast<unsigned>(ch - '0');
    return;
  }

  if (ch == '\'') {
    --len;
    type = Symbol::CHARACTER_LITERAL;
    return;
  }

  if (ch == '"') {
    type = Symbol::STRING_LITERAL;
    --len;
    return;
  }

  type = Symbol::UNKNOWN_IDENTIFIER;
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

  return error(ch, std::format("Unexpected character in identifier: ascii {}",
                               static_cast<int>(ch)));
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
          value = static_cast<unsigned>(*x);
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
      case Symbol::NUMBER_LITERAL:
        return finalize_NUMBER();
      case Symbol::CHARACTER_LITERAL:
        return finalize_CHARACTER();
      case Symbol::STRING_LITERAL:
        return finalize_STRING();
      case Symbol::UNKNOWN_IDENTIFIER:
        return finalize_IDENTIFIER();
      case Symbol::ERROR:
        return {Symbol::ERROR, str_to_bytes(identifier)};
      case Symbol::EOL:
        return {Symbol::EOL};
      case Symbol::END:
        return {Symbol::END};
      case Symbol::NONE:
        return {Symbol::NONE};
      case Symbol::REGISTER:
      case Symbol::TAG_DECL:
      case Symbol::TAG_REF:
      case Symbol::VERB:
      CASE_NONTERMINAL:
        assert(false);  // Unreachable code
        return {Symbol::ERROR};
    }

    assert(false);  // Unreachable code
    return {Symbol::ERROR};
  }();

  r.set_location(file_name, start_row, start_col);
  return r;
}

void TokenParser::clear() {
  start_row = row;
  start_col = col;
  len = 0;
  type = Symbol::NONE;
  num_base = 0;
  value = 0;
  prev_char = {};
  identifier = {};
}

Token TokenParser::finalize_NUMBER() {
  assert(value <= 0xffff);
  return {Symbol::NUMBER_LITERAL,
          {std::byte(value & 0xff), std::byte((value & 0xff00)>>8)}};
}

Token TokenParser::finalize_CHARACTER() {
  return {Symbol::CHARACTER_LITERAL,
          {std::byte(value & 0xff), std::byte((value & 0xff00)>>8)}};
}

Token TokenParser::finalize_STRING() {
  return {
      Symbol::STRING_LITERAL,
      str_to_bytes(identifier),
  };
}

Token TokenParser::finalize_IDENTIFIER() {
  if (const auto id = arch::from_string(identifier); id != Verb::ERROR) {
    return {
        Symbol::VERB,
        {std::byte(id & 0xff), std::byte{}},
    };
  }

  if (identifier.starts_with('r') && identifier.size() == 2 &&
      std::isdigit(identifier[1])) {
    return {Symbol::REGISTER,
            {std::byte(identifier[1] - '0'), std::byte(0x80)}};
  }

  if (identifier.ends_with(":")) {
    identifier.pop_back();
    return {Symbol::TAG_DECL, str_to_bytes(identifier)};
  }

  return {Symbol::TAG_REF, str_to_bytes(identifier)};
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
    if (token.symbol == Symbol::EOL || token.symbol == Symbol::ERROR ||
        token.symbol == Symbol::EOL) {
      out << '\n';
      firstchar = true;
    }
  }
  return out;
}