#pragma once

#include <format>
#include <iostream>
#include <stack>
#include <vector>

#include "grammar.hpp"

template <typename T>
concept TokenForwardIterator = requires(T t) {
                                 { t.operator*() } -> std::same_as<Token&>;
                                 { std::next(t) } -> std::same_as<T>;
                               };

bool parse(TokenForwardIterator auto begin, TokenForwardIterator auto end) {
  // Predeclaring inside the function to keep them unexported
  [[nodiscard]] bool production_rule(std::stack<Token> & stack,
                                     Token const& input);
  void cout_stack(std::stack<Token> stack, Token const& input);
  //

  std::stack<Token> stack{};
  stack.push({Symbol::Start});

  bool ok = true;
  auto it = begin;
  while (it != end) {
    cout_stack(stack, *it);

    if (ok = (*it).symbol != Symbol::ERROR; !ok) {
      break;
    }

    if (*it == stack.top()) {
      stack.pop();
      it = std::next(it);
      continue;
    }
    
    if (ok = production_rule(stack, *it); !ok) {
      break;
    }
  }

  if (!ok) {
    const Token t = *it;
    std::cerr << std::format(
        "Parsing error in line {}, column {}: Unexpected token {}\n", t.row(),
        t.col(), t.fmt());
    return false;
  }

  std::cout << "Success" << std::endl;
  return true;
}