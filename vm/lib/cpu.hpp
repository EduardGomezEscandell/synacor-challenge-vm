#pragma once

#include <iostream>
#include <ostream>

#include "memory.hpp"
#include "word.hpp"

namespace SynacorVM {

struct CPU {
  Memory &memory;
  std::ostream &terminal = std::cout;

  void Run() noexcept;

  bool Step();

  Number instruction_pointer = Number(0);
};

} // namespace SynacorVM