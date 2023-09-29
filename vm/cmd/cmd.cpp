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

std::basic_string<std::byte> read_binary(std::string file_name) {
  // Read binary file
  FILE *f = ::fopen(file_name.c_str(), "rb");
  if (f == nullptr) {
    throw std::runtime_error(std::format("Setup: could not read binary file {}", file_name));
  }

  ::fseek(f, 0, SEEK_END);
  auto fsize = std::size_t(::ftell(f));
  ::fseek(f, 0, SEEK_SET);

  std::basic_string<std::byte> buffer(fsize, std::byte(0));
  std::ignore = ::fread(buffer.data(), fsize, 1, f);
  ::fclose(f);

  return buffer;
}