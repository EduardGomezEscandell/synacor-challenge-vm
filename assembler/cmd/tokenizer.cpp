#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "lib/tokenizer.hpp"

void cout_tokenized(std::vector<Token> const&);

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Argc should be 2\n";
    exit(EXIT_FAILURE);
  }

  auto f = std::ifstream(argv[1], std::ios_base::binary);

  const auto out = tokenize(f);
  std::cout << "Tokenization:\n";
  cout_tokenized(out);

  f.close();

  return EXIT_SUCCESS;
}

void cout_tokenized(std::vector<Token> const& tokenized) {
  for (auto const& token : tokenized) {
    std::cout << token.fmt() << ' ';
    if (token.type == Token::EOL) {
      std::cout << '\n';
    }
  }
  std::cout << '\n';
}
