#pragma once

#include <iostream>
#include <ostream>

#include "memory.hpp"

namespace SynacorVM {

class CPU {
  Memory mem;
  Number instruction_pointer;
  std::ostream& terminal = std::cout;

 public:
  std::size_t Load(std::istream& in) { return mem.Load(in); }

  void Run() noexcept;

  void Step();
};

}  // namespace SynacorVM