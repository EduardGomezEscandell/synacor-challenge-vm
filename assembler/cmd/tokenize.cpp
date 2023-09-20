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

  auto f = std::ifstream(argv[1], std::ios_base::binary);

  const auto out = tokenize(f);
  std::cout << "Tokenization:\n";
  fmt_tokens(std::cout, out);

  f.close();

  return EXIT_SUCCESS;
}