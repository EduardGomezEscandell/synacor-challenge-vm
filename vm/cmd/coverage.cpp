#include <algorithm>
#include <array>
#include <bits/ranges_algo.h>
#include <cstdlib>
#include <format>
#include <fstream>
#include <initializer_list>
#include <ios>
#include <iostream>
#include <istream>
#include <numeric>

#include "arch/arch.hpp"

#include "lib/cpu.hpp"
#include "lib/memory.hpp"

#include "helpers.hpp"
#include "lib/word.hpp"

#include <csignal>

void handle(int);
void summary();

std::array<bool, SynacorVM::Memory::heap_size> visited;

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Argc should be 2\n";
    exit(EXIT_FAILURE);
  }

  ::signal(SIGINT, &handle);

  SynacorVM::Memory ram;

  SynacorVM::CPU vm{
      .memory = ram,
      .pre_exec_hook = [](SynacorVM::execution_state es) -> void {
        const SynacorVM::Word verb = es.heap[es.instruction_ptr.to_uint()];
        const auto v = static_cast<Verb>(verb.to_uint());
        const auto argc = arch::argument_count(v);
        if (argc < 0) {
          std::cerr << std::format("UNKNOWN {:x}\n", verb.to_uint());
          return;
        }

        const auto begin = visited.begin() + es.instruction_ptr.to_uint();
        const auto end = begin + 1 + argc;
        std::fill(begin, end, true);
      }};

  ram.load(read_binary(argv[1]));

  vm.Run();

  summary();
  return 0;
}

void summary() {
  auto visit_count = std::count(visited.begin(), visited.end(), 0);
  std::cerr << "\n-------------\n"<< std::format("Covered {} addresses ({:.2} %)\n", visit_count,
                           float(visit_count) / float(0x8000));
}

void handle(int) {
  summary();
  exit(EXIT_FAILURE);
}
