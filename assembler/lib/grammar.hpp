#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "arch/arch.hpp"

// Check out $root/docs/assembly/assembly.md#grammar
enum class Symbol : int {
  // Non-Terminals
  Start = -100,
  P,
  T,
  I,
  D,
  W,

  // Terminals

  // delimiters
  END,
  EOL,

  // literals
  NUMBER_LITERAL = 50,
  CHARACTER_LITERAL,
  STRING_LITERAL,

  // identifiers
  REGISTER,
  TAG_DECL,
  TAG_REF,
  VERB,


  // Temporary symbols used in parsing
  NONE = 0,
  UNKNOWN_IDENTIFIER,  // Identifier, not yet known what type
  ERROR,               // Erroneous input
};

struct Token { 
  constexpr Token(Symbol symbol = Symbol::NONE, std::vector<std::byte> data = {}) :
    symbol{symbol}, data{data}
  {}

  void set_location(std::string_view file, unsigned row, unsigned col) noexcept{
    this->m_file = file;
    this->m_row = row;
    this->m_col = col;
  }


constexpr bool operator==(Token other) const noexcept {
    return symbol == other.symbol && data == other.data;
  }

  std::string as_str() const;
  std::string fmt() const;
  std::string location() const;

  Verb as_opcode() const;
  unsigned as_number() const;
  char as_char() const;
  wchar_t as_wchar() const;

  Symbol symbol;
  std::vector<std::byte> data = {};

  private:
    std::string m_file;
    unsigned m_row;
    unsigned m_col;
};