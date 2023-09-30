#pragma once

#include <iostream>
#include <ostream>

#include "memory.hpp"
#include "word.hpp"

namespace SynacorVM {

struct CPU {
  Memory &memory;

  std::ostream &stdOut = std::cout;
  std::istream &stdIn = std::cin;

  void Run() noexcept;

  bool Step();

  Number instruction_pointer = Number(0);
};

} // namespace SynacorVM