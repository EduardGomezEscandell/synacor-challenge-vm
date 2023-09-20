#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "arch/arch.hpp"

// Check out $root/docs/assembly/assembly.md#grammar
enum class Symbol : int {
  // NonTerminals
  Start = -100,
  E,
  T,
  I,
  D,
  W,

  // Terminals
  END,

  // Literals
  NUMBER_LITERAL = 50,
  CHARACTER_LITERAL,
  STRING_LITERAL,

  // Identifiers
  REGISTER,
  TAG_DECL,
  TAG_REF,
  VERB,

  // End of line
  EOL,

  // Temporary symbols used in parsing
  NONE = 0,
  UNKNOWN_IDENTIFIER,  // Identifier, not yet known what type
  ERROR,               // Erroneous input
};

struct Token {
  std::string as_str() const;
  std::string fmt() const;

  Verb as_opcode() const;
  unsigned as_number() const;
  char as_char() const;
  wchar_t as_wchar() const;

  Symbol type;
  std::vector<std::byte> data = {};

  constexpr bool operator==(Token other) const noexcept {
    return type == other.type && data == other.data;
  }
};