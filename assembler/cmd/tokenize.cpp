#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "lib/tokenizer.hpp"
#include "lib/parser.hpp"

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Argc should be 2\n";
    exit(EXIT_FAILURE);
  }

  auto [tokens, t_success] = tokenize(argv[1]);
  if(!t_success) {
    return EXIT_FAILURE;
  }
  
  auto [ast, p_success] = parse(tokens.begin(), tokens.end());
  if(!p_success) {
    return EXIT_FAILURE;
  }

  std::cout << "AST:\n";
  fmt_tokens(std::cout, tokens);

  return EXIT_SUCCESS;
}