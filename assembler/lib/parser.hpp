#pragma once

#include <algorithm>
#include <format>
#include <iostream>
#include <iterator>
#include <memory>
#include <stack>
#include <vector>

#include "grammar.hpp"

struct Node {
  Token token;
  std::vector<std::unique_ptr<Node>> children = {};

  friend std::ostream& operator<<(std::ostream& os, Node const& n);

 private:
  std::ostream& fmt(std::ostream& os, unsigned depth) const;
};

template <typename T>
concept TokenForwardIterator = requires(T t) {
                                 { t.operator*() } -> std::same_as<Token&>;
                                 { std::next(t) } -> std::same_as<T>;
                               };

[[nodiscard]] bool production_rule(std::stack<Node*>& stack,
                                   Token const& input);

void cout_stack(std::stack<Node*> stack, Token const& input);

template <typename Iterable = std::initializer_list<Token>>
void apply_production_rule(std::stack<Node*>& stack, Iterable replace) {
  auto* top = stack.top();
  stack.pop();

  // Adding new symbols as children of popped symbol
  top->children.reserve(top->children.size() + replace.size());
  const auto rend = top->children.rbegin();
  std::transform(replace.begin(), replace.end(),
                 std::back_inserter(top->children), [](Token const& token) {
                   return std::make_unique<Node>(Node{.token = token});
                 });
  const auto rbegin = top->children.rbegin();

  // Adding new symbols to stack (in reverse)
  std::for_each(rbegin, rend,
                [&stack](std::unique_ptr<Node>& n) { stack.push(n.get()); });
}

// parse returns the root of the Abstract Syntax Tree, and a success flag.
std::pair<Node, bool> parse(TokenForwardIterator auto begin,
                            TokenForwardIterator auto end) {
  Node root{.token = Symbol::Start};
  std::stack<Node*> stack{};
  stack.push(&root);

  bool ok = true;
  auto it = begin;
  while (it != end) {
    cout_stack(stack, *it);

    if (ok = (*it).symbol != Symbol::ERROR; !ok) {
      break;
    }

    if (Node& top = *stack.top(); *it == top.token) {
      apply_production_rule(stack, {});
      it = std::next(it);
      continue;
    }

    if (ok = production_rule(stack, *it); !ok) {
      break;
    }
  }

  if (!ok) {
    const Token t = *it;
    std::cerr << std::format("{}: parsing error: unexpected token {}\n",
                             t.location(), t.fmt());
    return {std::move(root), false};
  }

  return {std::move(root), true};
}
