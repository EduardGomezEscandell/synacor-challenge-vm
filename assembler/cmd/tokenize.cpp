#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "lib/tokenizer.hpp"

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Argc should be 2\n";
    exit(EXIT_FAILURE);
  }

  const auto out = tokenize(argv[1]);
  std::cout << "Tokenization:\n";
  fmt_tokens(std::cout, out);

  return EXIT_SUCCESS;
}