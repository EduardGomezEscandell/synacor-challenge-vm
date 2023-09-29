#include "code_generation.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <execution>
#include <format>
#include <map>
#include <ostream>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <streambuf>
#include <string>

#include "assembler/lib/grammar.hpp"
#include "assembler/lib/parser.hpp"

struct reference {
  std::vector<std::size_t> locations{};
  std::array<std::byte, 2> value = {};

  // Error-message helpers
  Node const *declaration = nullptr;
  Node const *first_usage = nullptr;
};

struct generator {
  std::map<std::string, reference> references{};

  std::basic_stringstream<std::byte> os = {};
  std::size_t write_ptr = 0;

  void process_nonterminal(std::stack<Node const *> &stack, Node const *curr);
  void generate_literal(Node const *terminal);

  void process_tag_declaration(Node const *n);
  void process_reference(Node const *n);
  void process_endline(Node const *n);
};

// The first pass traverses the AST depth-first and generates all the code,
// except where tag references exist. The locations of these references are
// stored and a place-holder value is written. The tag declarations and their
// values are also stored.
generator generate_without_references(Node const &root);

// The second pass replaces all reference placeholders with their values.
bytestr replace_references(generator &);

std::pair<bytestr, bool> generate(Node const &root) {
  try {
    auto g = generate_without_references(root);
    auto s = replace_references(g);
    return {s, true};
  } catch (std::runtime_error &e) {
    std::cerr << e.what() << std::endl;
    return {{}, false};
  } catch (...) {
    std::cerr << "Unknown error" << std::endl;
    return {{}, false};
  }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

generator generate_without_references(Node const &root) {
  generator g{};
  std::stack<Node const *> stack = {};

  stack.push(&root);

  while (true) {
    if (stack.empty()) {
      throw std::runtime_error("Incomplete AST");
    }

    Node const *curr = stack.top();
    stack.pop();

    if (curr == nullptr) {
      throw std::runtime_error("Null node in AST");
    }

    switch (curr->token.symbol) {
    case Symbol::END:
      return g;
    case Symbol::EOL:
      g.process_endline(curr);
      break;
    case Symbol::TAG_DECL:
      g.process_tag_declaration(curr);
      break;
    case Symbol::TAG_REF:
      g.process_reference(curr);
      break;
    case Symbol::NUMBER_LITERAL:
    case Symbol::CHARACTER_LITERAL:
    case Symbol::STRING_LITERAL:
    case Symbol::REGISTER:
    case Symbol::VERB:
      g.generate_literal(curr);
      break;
    CASE_NONTERMINAL:
      g.process_nonterminal(stack, curr);
      break;
    CASE_ERRONEOUS:
      throw std::runtime_error(
          std::format("AST contains error token {}", curr->token.fmt()));
    }
  }
}

#pragma GCC diagnostic pop

bytestr replace_references(generator &g) {
  bytestr s = std::move(g.os).str();
  g.os = {};

  for (auto &kv : g.references) {
    std::string_view name = kv.first;
    reference const &ref = kv.second;
    if (ref.declaration == nullptr) {
      throw std::runtime_error(std::format("{}: code generation error: reference {} is undefined.",
                                           ref.first_usage->token.location(),
                                           name));
    }

    if (ref.locations.empty()) {
      std::cerr << std::format("{}: Warning. Reference {} is unused.",
                               ref.declaration->token.location(), name)
                << std::endl;
    }

    for (const std::size_t loc : ref.locations) {
      // loc is multiplied by two because loc is the 16-bit
      // address, but bytestr uses 8-bit addressing.
      s[loc] = ref.value[0];
      s[loc + 1] = ref.value[1];
    }
  }

  return s;
}

void generator::process_nonterminal(std::stack<Node const *> &stack,
                                    Node const *curr) {
  std::for_each(curr->children.crbegin(), curr->children.crend(),
                [&stack](auto const &child) { stack.push(child.get()); });
}

void generator::generate_literal(Node const *terminal) {
  std::ranges::copy(terminal->token.data,
                    std::ostream_iterator<std::byte, std::byte>(os));

  // Ensuring alignment
  std::size_t len = terminal->token.data.size();
  if (len % 2 != 0) {
    os << std::byte(0);
    ++len;
  }

  write_ptr += len / 2;
}

void generator::process_tag_declaration(Node const *n) {
  assert(n->token.symbol == Symbol::TAG_DECL);

  std::string name = n->token.as_str();
  auto &ref = references[name];

  if (ref.declaration != nullptr) {
    throw std::runtime_error(std::format(
        "{}: Reference {} declared twice\n  Previous declaration: {}",
        n->token.fmt(), name, ref.declaration->token.fmt()));
  }

  ref.declaration = n;

  const auto idx = write_ptr;
  const auto lo = std::byte(idx & 0xff);
  const auto hi = std::byte((idx >> 8) & 0xff);
  ref.value = {lo, hi};
}

void generator::process_reference(Node const *n) {
  assert(n->token.symbol == Symbol::TAG_REF);
  auto &ref = references[n->token.as_str()];
  if (ref.first_usage == nullptr) {
    ref.first_usage = n;
  }
  ref.locations.push_back(write_ptr * 2);

  // Placeholder value
  os << std::byte(0xff) << std::byte(0xff);
  ++write_ptr;
}

void generator::process_endline(Node const *) {}
