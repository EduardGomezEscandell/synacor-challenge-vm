#pragma once

#include <cassert>
#include <cstdint>
#include <string_view>

enum Verb : std::int32_t {
  HALT,  // `halt`:   stop execution and terminate the program
  SET,   // `set a b`:   set register `a` to the value of `b`
  PUSH,  // `push a`:   push `a` onto the stack
  POP,   // `pop a`:   remove the top element from the stack and write it into
         // `a`; empty stack = error
  EQ,    // `eq a b c`:   set `a` to 1 if `b` is equal to `c`; set it to 0
         // otherwise
  GT,    // `gt a b c`:   set `a` to 1 if `b` is greater than `c`; set it to 0
         // otherwise
  JMP,   // `jmp a`:   jump to `a`
  JT,    // `jt a b`:   if `a` is nonzero, jump to `b`
  JF,    // `jf a b`:   if `a` is zero, jump to `b`
  ADD,   // `add a b c`:   assign into `a` the sum of `b` and `c` (modulo 32768)
  MULT,  // `mult a b c`:   store into `a` the product of `b` and `c` (modulo
         // 32768)
  MOD,   // `mod a b c`:   store into `a` the remainder of `b` divided by `c`
  AND,   // `and a b c`:   stores into `a` the bitwise and of `b` and `c`
  OR,    // `or a b c`:   stores into `a` the bitwise or of `b` and `c`
  NOT,   // `not a b`:   stores 15-bit bitwise inverse of `b` in `a`
  RMEM,  // `rmem a b`:   read memory at address `b` and write it to `a`
  WMEM,  // `wmem a b`:   write the value from `b` into memory at address `a`
  CALL,  // `call a`:   write the address of the next instruction to the stack
         // and jump to `a`
  RET,   // `ret`:   remove the top element from the stack and jump to it; empty
         // stack = halt
  OUT,   // `out a`:   write the character represented by ascii code `a` to the
         // terminal
  IN,  // `in a`:   read a character from the terminal and write its ascii code
       // to `a`; it can be assumed that once input starts, it will continue
       // until a newline is encountered; this means that you can safely read
       // whole lines from the keyboard instead of having to figure out how to
       // read individual characters
  NOOP,  // `noop`:   no operation

  ERROR,  // `error`: Erroneus instruction. Will immediately halt execution.
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