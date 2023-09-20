#pragma once

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

constexpr std::string_view to_string(Verb v) {
  switch (v) {
    case HALT:
      return "halt";
    case SET:
      return "set";
    case PUSH:
      return "push";
    case POP:
      return "pop";
    case EQ:
      return "eq";
    case GT:
      return "gt";
    case JMP:
      return "jmp";
    case JT:
      return "jt";
    case JF:
      return "jf";
    case ADD:
      return "add";
    case MULT:
      return "mult";
    case MOD:
      return "mod";
    case AND:
      return "and";
    case OR:
      return "or";
    case NOT:
      return "not";
    case RMEM:
      return "rmem";
    case WMEM:
      return "wmem";
    case CALL:
      return "call";
    case RET:
      return "ret";
    case OUT:
      return "out";
    case IN:
      return "in";
    case NOOP:
      return "noop";
    case ERROR:
      return "ERROR";
  }

  return "ERROR";
}

constexpr int argument_count(Verb v) {
  switch (v) {
    case HALT:
      return 0;
    case SET:
      return 2;
    case PUSH:
      return 1;
    case POP:
      return 1;
    case EQ:
      return 3;
    case GT:
      return 3;
    case JMP:
      return 1;
    case JT:
      return 2;
    case JF:
      return 2;
    case ADD:
      return 3;
    case MULT:
      return 3;
    case MOD:
      return 3;
    case AND:
      return 3;
    case OR:
      return 3;
    case NOT:
      return 2;
    case RMEM:
      return 2;
    case WMEM:
      return 2;
    case CALL:
      return 1;
    case RET:
      return 0;
    case OUT:
      return 1;
    case IN:
      return 1;
    case NOOP:
      return 0;
    case ERROR:
      return 0;
  }
}


}