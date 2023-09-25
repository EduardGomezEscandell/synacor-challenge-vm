
#include "parser.hpp"

#include <algorithm>
#include <format>
#include <initializer_list>
#include <iterator>
#include <memory>

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

[[nodiscard]] bool rule_Start(std::stack<Node*>& stack, Token const& input);
[[nodiscard]] bool rule_P(std::stack<Node*>& stack, Token const& input);
[[nodiscard]] bool rule_T(std::stack<Node*>& stack, Token const& input);
[[nodiscard]] bool rule_I(std::stack<Node*>& stack, Token const& input);
[[nodiscard]] bool rule_D(std::stack<Node*>& stack, Token const& input);
[[nodiscard]] bool rule_W(std::stack<Node*>& stack, Token const& input);

bool production_rule(std::stack<Node*>& stack, Token const& input) {
  auto const& top = *stack.top();
  switch (top.token.symbol) {
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

bool rule_Start(std::stack<Node*>& stack, Token const& input) {
  [[maybe_unused]] const auto& top = *stack.top();
  assert(top.token.symbol == Symbol::Start);

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
      apply_production_rule(stack, {Symbol::P, Symbol::END});
      return true;
    CASE_NONTERMINAL:
      return false;
    CASE_ERRONEOUS:
      break;
  }
  assert(0);
  return false;
}

bool rule_P(std::stack<Node*>& stack, Token const& input) {
  [[maybe_unused]] const auto& top = *stack.top();
  assert(top.token.symbol == Symbol::P);

  switch (input.symbol) {
    case Symbol::END:
      apply_production_rule(stack, {Symbol::END});
      return true;
    case Symbol::NUMBER_LITERAL:
    case Symbol::CHARACTER_LITERAL:
    case Symbol::STRING_LITERAL:
    case Symbol::REGISTER:
    case Symbol::TAG_REF:
      apply_production_rule(stack, {Symbol::D, Symbol::EOL, Symbol::P});
      return true;
    case Symbol::TAG_DECL:
      apply_production_rule(stack, {Symbol::T, Symbol::EOL, Symbol::P});
      return true;
    case Symbol::VERB:
      apply_production_rule(stack, {Symbol::I, Symbol::EOL, Symbol::P});
      return true;
    case Symbol::EOL:
      apply_production_rule(stack, {Symbol::EOL, Symbol::P});
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

bool rule_T(std::stack<Node*>& stack, Token const& input) {
  [[maybe_unused]] const auto& top = *stack.top();
  assert(top.token.symbol == Symbol::T);

  switch (input.symbol) {
    case Symbol::TAG_DECL:
      apply_production_rule(stack, {input});
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

bool rule_I(std::stack<Node*>& stack, Token const& input) {
  [[maybe_unused]] const auto& top = *stack.top();
  assert(top.token.symbol == Symbol::I);

  switch (input.symbol) {
    case Symbol::VERB: {
      const auto argc = arch::argument_count(input.as_opcode());
      std::vector<Token> w{input};
      w.resize(1 + std::size_t(argc), {Symbol::W});
      apply_production_rule(stack, w);
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

bool rule_D(std::stack<Node*>& stack, Token const& input) {
  [[maybe_unused]] const auto& top = *stack.top();
  assert(top.token.symbol == Symbol::D);

  switch (input.symbol) {
    case Symbol::NUMBER_LITERAL:
    case Symbol::CHARACTER_LITERAL:
    case Symbol::STRING_LITERAL:
    case Symbol::REGISTER:
    case Symbol::TAG_REF:
      apply_production_rule(stack, {input, Symbol::D});
      return true;
    case Symbol::EOL:
      apply_production_rule(stack, {});
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

bool rule_W(std::stack<Node*>& stack, Token const& input) {
  [[maybe_unused]] const auto& top = *stack.top();
  assert(top.token.symbol == Symbol::W);

  switch (input.symbol) {
    case Symbol::END:
    case Symbol::NUMBER_LITERAL:
    case Symbol::CHARACTER_LITERAL:
    case Symbol::REGISTER:
    case Symbol::TAG_REF:
      apply_production_rule(stack, {input});
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

void cout_stack(std::stack<Node*> stack, Token const& in) {
  std::cout << std::format("Parsing stack with input: {}\n", in.fmt());
  while (stack.size() != 0) {
    std::cout << stack.top()->token.fmt() << '\n';
    stack.pop();
  }
  std::cout << "\n\n";
}

#else
void cout_stack(std::stack<Node*>, Token const&) {}

#endif

std::ostream& operator<<(std::ostream& os, Node const& n) {
  return n.fmt(os, 0);
}

std::ostream& Node::fmt(std::ostream& os, unsigned depth) const {
  std::string prefix(2 * depth, ' ');
  os << prefix << token.fmt() << '\n';
  for (auto& ch : children) {
    ch->fmt(os, depth + 1);
  }
  return os;
}

#pragma GCC diagnostic pop