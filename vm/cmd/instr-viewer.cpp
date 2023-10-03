#include <array>
#include <cstdlib>
#include <format>
#include <fstream>
#include <initializer_list>
#include <ios>
#include <iostream>
#include <istream>

#include "arch/arch.hpp"

#include "lib/cpu.hpp"
#include "lib/memory.hpp"

#include "helpers.hpp"
#include "lib/word.hpp"

std::basic_string<std::byte> read_binary(std::string file_name);

void pre_exec_hook(SynacorVM::execution_state es);

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Argc should be 2\n";
    exit(EXIT_FAILURE);
  }

  SynacorVM::Memory ram;

  SynacorVM::CPU vm{.memory = ram, .pre_exec_hook = pre_exec_hook};

  ram.load(read_binary(argv[1]));

  vm.Run();

  return 0;
}

std::string parse_value(SynacorVM::Word w) {
  const auto v = w.to_uint();
  if (v >= 0x8000 && v < 0x8008) {
    // Register
    return std::format("r{}", static_cast<int>(v - 0x8000));
  }

  if (v >= ' ' && v <= '~') {
    // Literal character
    return std::format("'{}'", static_cast<char>(v));
  }

  return std::format("{:x}", v);
}

void pre_exec_hook(SynacorVM::execution_state es) {

  const SynacorVM::Word verb = es.heap[es.instruction_ptr.to_uint()];
  
  const auto v = static_cast<Verb>(verb.to_uint());

  std::cerr << std::format("0x{:04x} | {}", es.instruction_ptr.to_uint(), arch::to_string(v));
  const auto argc = arch::argument_count(v);
  if (argc < 0) {
    std::cerr << std::format("UNKNOWN {:x}\n", verb.to_uint());
    return;
  }

  for (auto i = 0u; i < unsigned(argc); ++i) {
    std::cerr << ' '
              << parse_value(es.heap[es.instruction_ptr.to_uint() + 1 + i]);
  }

  std::cerr << '\n';
}