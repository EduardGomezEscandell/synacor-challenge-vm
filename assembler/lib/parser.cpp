
#include "parser.hpp"

#include <format>

#include "arch/arch.hpp"
#include "grammar.hpp"
#include "tokenizer.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

#define CASE_ERRONEOUS             \
  case Symbol::NONE:               \
  case Symbol::UNKNOWN_IDENTIFIER: \
  case Symbol::ERROR

#define CASE_NONTERMINAL \
  case Symbol::Start:    \
  case Symbol::P:        \
  case Symbol::T:        \
  case Symbol::I:        \
  case Symbol::D:        \
  case Symbol::W

#define CASE_TERMINAL             \
  case Symbol::END:               \
  case Symbol::NUMBER_LITERAL:    \
  case Symbol::CHARACTER_LITERAL: \
  case Symbol::STRING_LITERAL:    \
  case Symbol::REGISTER:          \
  case Symbol::TAG_DECL:          \
  case Symbol::TAG_REF:           \
  case Symbol::VERB:              \
  case Symbol::EOL

[[nodiscard]] bool rule_Start(std::stack<Token>& stack, Token const& input);
[[nodiscard]] bool rule_P(std::stack<Token>& stack, Token const& input);
[[nodiscard]] bool rule_T(std::stack<Token>& stack, Token const& input);
[[nodiscard]] bool rule_I(std::stack<Token>& stack, Token const& input);
[[nodiscard]] bool rule_D(std::stack<Token>& stack, Token const& input);
[[nodiscard]] bool rule_W(std::stack<Token>& stack, Token const& input);

bool production_rule(std::stack<Token>& stack, Token const& input) {
  auto const& top = stack.top();
  switch (top.symbol) {
    case Symbol::Start:
      return rule_Start(stack, input);
    case Symbol::P:
      return rule_P(stack, input);
    case Symbol::T:
      return rule_T(stack, input);
    case Symbol::I:
      return rule_I(stack, input);
    case Symbol::D:
      return rule_D(stack, input);
    case Symbol::W:
      return rule_W(stack, input);
    CASE_TERMINAL:
    CASE_ERRONEOUS:
      break;
  }

  assert(0);
  return false;
}

bool rule_Start(std::stack<Token>& stack, Token const& input) {
  [[maybe_unused]] const auto& top = stack.top();
  assert(top.symbol == Symbol::Start);

  switch (input.symbol) {
    case Symbol::END:
    case Symbol::NUMBER_LITERAL:
    case Symbol::CHARACTER_LITERAL:
    case Symbol::STRING_LITERAL:
    case Symbol::REGISTER:
    case Symbol::TAG_DECL:
    case Symbol::TAG_REF:
    case Symbol::VERB:
    case Symbol::EOL:
      stack.pop();
      stack.push({Symbol::END});
      stack.push({Symbol::P});
      return true;
    CASE_NONTERMINAL:
      return false;
    CASE_ERRONEOUS:
      break;
  }
  assert(0);
  return false;
}

bool rule_P(std::stack<Token>& stack, Token const& input) {
  [[maybe_unused]] const auto& top = stack.top();
  assert(top.symbol == Symbol::P);

  switch (input.symbol) {
    case Symbol::END:
      stack.pop();
      stack.push({Symbol::END});
      return true;
    case Symbol::NUMBER_LITERAL:
    case Symbol::CHARACTER_LITERAL:
    case Symbol::STRING_LITERAL:
    case Symbol::REGISTER:
    case Symbol::TAG_REF:
      stack.pop();
      stack.push({Symbol::P});
      stack.push({Symbol::EOL});
      stack.push({Symbol::D});
      return true;
    case Symbol::TAG_DECL:
      stack.pop();
      stack.push({Symbol::P});
      stack.push({Symbol::EOL});
      stack.push({Symbol::T});
      return true;
    case Symbol::VERB:
      stack.pop();
      stack.push({Symbol::P});
      stack.push({Symbol::EOL});
      stack.push({Symbol::I});
      return true;
    case Symbol::EOL:
      stack.pop();
      stack.push({Symbol::P});
      stack.push({Symbol::END});
      return true;
    CASE_NONTERMINAL:
      // Bad grammar
      return false;
    CASE_ERRONEOUS:
      break;
  }
  assert(0);
  return false;
}

bool rule_T(std::stack<Token>& stack, Token const& input) {
  [[maybe_unused]] const auto& top = stack.top();
  assert(top.symbol == Symbol::T);

  switch (input.symbol) {
    case Symbol::TAG_DECL:
      stack.pop();
      stack.push(input);
      return true;
    case Symbol::END:
    case Symbol::NUMBER_LITERAL:
    case Symbol::CHARACTER_LITERAL:
    case Symbol::STRING_LITERAL:
    case Symbol::REGISTER:
    case Symbol::TAG_REF:
    case Symbol::VERB:
    case Symbol::EOL:
    CASE_NONTERMINAL:
      // Bad grammar
      return false;
    CASE_ERRONEOUS:
      break;
  }
  assert(0);
  return false;
}

bool rule_I(std::stack<Token>& stack, Token const& input) {
  [[maybe_unused]] const auto& top = stack.top();
  assert(top.symbol == Symbol::I);

  switch (input.symbol) {
    case Symbol::VERB: {
      stack.pop();
      const auto argc = arch::argument_count(input.as_opcode());
      for (int i = 0; i < argc; ++i) {
        stack.push({Symbol::W});
      }
      stack.push(input);
      return true;
    }
    case Symbol::END:
    case Symbol::NUMBER_LITERAL:
    case Symbol::CHARACTER_LITERAL:
    case Symbol::STRING_LITERAL:
    case Symbol::REGISTER:
    case Symbol::TAG_DECL:
    case Symbol::TAG_REF:
    case Symbol::EOL:
    CASE_NONTERMINAL:
      // Bad grammar
      return false;
    CASE_ERRONEOUS:
      break;
  }
  assert(0);
  return false;
}

bool rule_D(std::stack<Token>& stack, Token const& input) {
  [[maybe_unused]] const auto& top = stack.top();
  assert(top.symbol == Symbol::D);

  switch (input.symbol) {
    case Symbol::NUMBER_LITERAL:
    case Symbol::CHARACTER_LITERAL:
    case Symbol::STRING_LITERAL:
    case Symbol::REGISTER:
    case Symbol::TAG_REF:
      stack.pop();
      stack.push({Symbol::D});
      stack.push(input);
      return true;
    case Symbol::EOL:
      stack.pop();
      return true;
    case Symbol::END:
    case Symbol::TAG_DECL:
    case Symbol::VERB:
    CASE_NONTERMINAL:
      // Bad grammar
      return false;
    CASE_ERRONEOUS:
      break;
  }

  assert(0);
  return false;
}

bool rule_W(std::stack<Token>& stack, Token const& input) {
  [[maybe_unused]] const auto& top = stack.top();
  assert(top.symbol == Symbol::W);

  switch (input.symbol) {
    case Symbol::END:
    case Symbol::NUMBER_LITERAL:
    case Symbol::CHARACTER_LITERAL:
    case Symbol::REGISTER:
    case Symbol::TAG_REF:
      stack.pop();
      stack.push(input);
      return true;
    case Symbol::STRING_LITERAL:
    case Symbol::TAG_DECL:
    case Symbol::VERB:
    case Symbol::EOL:
    CASE_NONTERMINAL:
      // Bad grammar
      return false;
    CASE_ERRONEOUS:
      break;
  }
  assert(0);
  return false;
}

#ifndef NDEBUG

void cout_stack(std::stack<Token> stack, Token const& in) {
  std::cout << std::format("Parsing stack with input: {}\n", in.fmt());
  while (stack.size() != 0) {
    std::cout << stack.top().fmt() << '\n';
    stack.pop();
  }
  std::cout << "\n\n";
}

#else
void cout_stack(std::stack<Token>, Token const&) {}

#endif

#pragma GCC diagnostic pop