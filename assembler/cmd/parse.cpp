#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "lib/parser.hpp"
#include "lib/tokenizer.hpp"

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Argc should be 2\n";
    exit(EXIT_FAILURE);
  }

  auto tokenized = tokenize({argv[1]});
  parse(tokenized.begin(), tokenized.end());

  return EXIT_SUCCESS;
}