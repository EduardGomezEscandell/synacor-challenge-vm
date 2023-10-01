#include "grammar.hpp"

#include <cassert>
#include <format>
#include <string>

std::string Token::location() const {
  return std::format("{}:{}:{}", m_file, m_row, m_col);
}

std::string Token::fmt() const {
  switch (symbol) {
    // Non-terminals
    case Symbol::Start:
      return "<Start>";
    case Symbol::END:
      return "<END>";
    case Symbol::P:
      return "<P>";
    case Symbol::T:
      return "<T>";
    case Symbol::I:
      return "<I>";
    case Symbol::D:
      return "<D>";
    case Symbol::W:
      return "<W>";
    case Symbol::R:
      return "<R>";
    // Terminals
    case Symbol::NUMBER_LITERAL:
      return std::format("<NUMBER {}>", as_number());
    case Symbol::CHARACTER_LITERAL:
      return std::format("<CHARACTER {}>", as_char());
    case Symbol::STRING_LITERAL:
      return std::format("<STRING {}>", as_str());
    case Symbol::EOL:
      return "<EOL>";
    case Symbol::REGISTER:
      return std::format("<REGISTER {}>", data.empty() ? -1 : int(data[0]));
    case Symbol::TAG_DECL:
      return std::format("<TAG_DECL {}>", as_str());
    case Symbol::TAG_REF:
      return std::format("<TAG_REF {}>", as_str());
    case Symbol::VERB:
      return std::format("<VERB {}>", arch::to_string(as_opcode()));
    // Special tokens
    case Symbol::NONE:
      return "{NONE}";
    case Symbol::UNKNOWN_IDENTIFIER:
      return std::format("<UNKNOWN_IDENTIFIER {}>", as_str());
    case Symbol::ERROR:
      return std::format("<ERROR {}>", as_str());
  }

  assert(false);  // Unreachable code
  return "<ERROR BAD CODE!>";
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

Verb Token::as_opcode() const {
  assert(symbol == Symbol::VERB);
  const auto n = as_number();
  return static_cast<Verb>(n);
}

char Token::as_char() const { return static_cast<char>(as_number()); }
