#include <array>
#include <cstdlib>
#include <format>
#include <fstream>
#include <initializer_list>
#include <ios>
#include <iostream>
#include <istream>

#include "lib/cpu.hpp"
#include "lib/memory.hpp"

#include "helpers.hpp"

std::basic_string<std::byte> read_binary(std::string file_name);

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Argc should be 2\n";
    exit(EXIT_FAILURE);
  }

  SynacorVM::Memory ram;

  SynacorVM::CPU vm{.memory = ram};

  ram.load(read_binary(argv[1]));

  vm.Run();

  return 0;
}

