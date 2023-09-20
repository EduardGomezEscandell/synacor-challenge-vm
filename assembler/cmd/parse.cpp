#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "lib/parser.hpp"
#include "lib/tokenizer.hpp"


int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Argc should be 2\n";
    exit(EXIT_FAILURE);
  }

  auto f = std::ifstream(argv[1], std::ios_base::binary);
  auto tokenized = tokenize(f);
  f.close();

  parse(tokenized.begin(), tokenized.end());

  return EXIT_SUCCESS;
}