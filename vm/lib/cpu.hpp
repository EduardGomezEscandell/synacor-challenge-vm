#pragma once

#include <functional>
#include <iostream>
#include <ostream>
#include <stack>

#include "memory.hpp"
#include "word.hpp"

namespace SynacorVM {

struct CPU;

struct execution_state {
  constexpr explicit execution_state(CPU const& cpu);
  
  Number instruction_ptr;
  std::array<Word, Memory::register_count> const &registers;
  std::array<Word, Memory::heap_size> const &heap;
  std::stack<Word> const &stack;
};

struct CPU {
  Memory &memory;

  std::ostream &stdOut = std::cout;
  std::istream &stdIn = std::cin;

  void Run() noexcept;

  bool Step();

  Number instruction_pointer = Number(0);

  friend struct execution_state;
  std::function<void(execution_state)> pre_exec_hook = nullptr;
  std::function<void(execution_state,bool)> post_exec_hook = nullptr;
};

constexpr execution_state::execution_state(CPU const& cpu)
      : instruction_ptr(cpu.instruction_pointer), registers(cpu.memory.m_registers),
        heap(cpu.memory.m_heap), stack(cpu.memory.m_stack) {}

} // namespace SynacorVM