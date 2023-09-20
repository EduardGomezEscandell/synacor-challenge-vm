#pragma once

#include <cassert>
#include <cstdint>
#include <string_view>

enum Verb : std::int32_t {
  HALT,
  SET,
  PUSH,
  POP,
  EQ,
  GT,
  JMP,
  JT,
  JF,
  ADD,
  MULT,
  MOD,
  AND,
  OR,
  NOT,
  RMEM,
  WMEM,
  CALL,
  RET,
  OUT,
  IN,
  NOOP,
  ERROR,
};

namespace arch {

constexpr Verb from_string(std::string_view s) {
  if (s == "halt") return HALT;
  if (s == "set") return SET;
  if (s == "push") return PUSH;
  if (s == "pop") return POP;
  if (s == "eq") return EQ;
  if (s == "gt") return GT;
  if (s == "jmp") return JMP;
  if (s == "jt") return JT;
  if (s == "jf") return JF;
  if (s == "add") return ADD;
  if (s == "mult") return MULT;
  if (s == "mod") return MOD;
  if (s == "and") return AND;
  if (s == "or") return OR;
  if (s == "not") return NOT;
  if (s == "rmem") return RMEM;
  if (s == "wmem") return WMEM;
  if (s == "call") return CALL;
  if (s == "ret") return RET;
  if (s == "out") return OUT;
  if (s == "in") return IN;
  if (s == "noop") return NOOP;

  return ERROR;
}

#define JUMPTABLE_ROW(k, v) \
  case k:                   \
    return v;

constexpr std::string_view to_string(Verb v) {
  switch (v) {
    JUMPTABLE_ROW(HALT, "halt")
    JUMPTABLE_ROW(SET, "set")
    JUMPTABLE_ROW(PUSH, "push")
    JUMPTABLE_ROW(POP, "pop")
    JUMPTABLE_ROW(EQ, "eq")
    JUMPTABLE_ROW(GT, "gt")
    JUMPTABLE_ROW(JMP, "jmp")
    JUMPTABLE_ROW(JT, "jt")
    JUMPTABLE_ROW(JF, "jf")
    JUMPTABLE_ROW(ADD, "add")
    JUMPTABLE_ROW(MULT, "mult")
    JUMPTABLE_ROW(MOD, "mod")
    JUMPTABLE_ROW(AND, "and")
    JUMPTABLE_ROW(OR, "or")
    JUMPTABLE_ROW(NOT, "not")
    JUMPTABLE_ROW(RMEM, "rmem")
    JUMPTABLE_ROW(WMEM, "wmem")
    JUMPTABLE_ROW(CALL, "call")
    JUMPTABLE_ROW(RET, "ret")
    JUMPTABLE_ROW(OUT, "out")
    JUMPTABLE_ROW(IN, "in")
    JUMPTABLE_ROW(NOOP, "noop")
    JUMPTABLE_ROW(ERROR, "ERROR")
  }

  return "ERROR";
}

constexpr int argument_count(Verb v) {
  switch (v) {
    JUMPTABLE_ROW(HALT, 0)
    JUMPTABLE_ROW(SET, 2)
    JUMPTABLE_ROW(PUSH, 1)
    JUMPTABLE_ROW(POP, 1)
    JUMPTABLE_ROW(EQ, 3)
    JUMPTABLE_ROW(GT, 3)
    JUMPTABLE_ROW(JMP, 1)
    JUMPTABLE_ROW(JT, 2)
    JUMPTABLE_ROW(JF, 2)
    JUMPTABLE_ROW(ADD, 3)
    JUMPTABLE_ROW(MULT, 3)
    JUMPTABLE_ROW(MOD, 3)
    JUMPTABLE_ROW(AND, 3)
    JUMPTABLE_ROW(OR, 3)
    JUMPTABLE_ROW(NOT, 2)
    JUMPTABLE_ROW(RMEM, 2)
    JUMPTABLE_ROW(WMEM, 2)
    JUMPTABLE_ROW(CALL, 1)
    JUMPTABLE_ROW(RET, 0)
    JUMPTABLE_ROW(OUT, 1)
    JUMPTABLE_ROW(IN, 1)
    JUMPTABLE_ROW(NOOP, 0)
    case ERROR:
      break;
  }

  assert(0);
  return -1;
}

#undef JUMPTABLE_ROW

}  // namespace arch