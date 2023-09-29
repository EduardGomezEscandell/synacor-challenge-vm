#include <bits/types/FILE.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>
#include <vector>

#include "lib/code_generation.hpp"
#include "lib/parser.hpp"
#include "lib/tokenizer.hpp"

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Argc should be 3:\n\tassemble <INPUT_FILE> <OUTPUT_FILE>\n";
    exit(EXIT_FAILURE);
  }

  auto [tokens, t_success] = tokenize(argv[1]);
  if (!t_success) {
    return EXIT_FAILURE;
  }

  auto [ast, p_success] = parse(tokens.begin(), tokens.end());
  if (!p_success) {
    return EXIT_FAILURE;
  }

  auto [code, cg_success] = generate(ast);
  if (!cg_success) {
    return EXIT_FAILURE;
  }

  ::FILE *f = ::fopen(argv[2], "wb");
  if(f == nullptr) {
    std::cerr << "Failed to open outfile" << std::endl;
    return EXIT_FAILURE;
  }

  if(auto n = ::fwrite(code.data(), code.size(), 1, f); n != 1) {
    std::cerr << "Failed to write outfile" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}