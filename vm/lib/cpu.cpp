#include "cpu.hpp"

#include <exception>
#include <format>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string_view>

#include "arch/arch.hpp"

namespace SynacorVM {

Word *in_prt(Memory &m, Number ptr) {
  const Word v = m[ptr];
  return &m[v];
}

Word out_ptr(Memory &m, Number ptr) {
  const Word v = m[ptr];
  if (v < Memory::heap_size) {
    return v;
  }

  return m[v];
}

bool CPU::Step() {
  auto opcode = Number(memory[instruction_pointer++].to_uint());
  switch (opcode.to_int()) {
    case HALT:
      return false;
    case SET: {
      Word *const aptr = in_prt(memory, instruction_pointer++);
      Word b = out_ptr(memory, instruction_pointer++);
      *aptr = b;
      return true;
    }
    case PUSH: {
      const Word w = out_ptr(memory, instruction_pointer++);
      memory.push(w);
      return true;
    }
    case POP: {
      Word *const ptr = in_prt(memory, instruction_pointer++);
      if(memory.stack_ptr() == 0) {
        throw std::runtime_error("Called POP with an empty stack");
      }
      *ptr = memory.pop();
      return true;
    }
    case EQ: {
      Word *const aptr = in_prt(memory, instruction_pointer++);
      Word b = out_ptr(memory, instruction_pointer++);
      Word c = out_ptr(memory, instruction_pointer++);
      *aptr = (b == c) ? Word(1) : Word(0);
      return true;
    }
    case GT: {
      Word *const aptr = in_prt(memory, instruction_pointer++);
      Word b = out_ptr(memory, instruction_pointer++);
      Word c = out_ptr(memory, instruction_pointer++);
      *aptr = (b > c) ? Word(1) : Word(0);
      return true;
    }
    case JMP: {
      Word pos = out_ptr(memory, instruction_pointer++);
      if (pos > Memory::heap_size) {
        throw std::runtime_error(std::format(
            "Attempted to move instruction pointer to non-existing address {}",
            pos.to_uint()));
      }
      instruction_pointer = Number(pos);
      return true;
    }
    case JT: {
      Word cond = out_ptr(memory, instruction_pointer++);
      Word pos = out_ptr(memory, instruction_pointer++);
      if (!cond.nonzero()) {
        return true;
      }
      if (pos > Memory::heap_size) {
        throw std::runtime_error(
            std::format("Attempted to move instruction pointer to "
                        "non-existing address {}",
                        pos.to_uint()));
      }
      instruction_pointer = Number(pos);
      return true;
    }
    case JF: {
      Word cond = out_ptr(memory, instruction_pointer++);
      Word pos = out_ptr(memory, instruction_pointer++);
      if (cond.nonzero()) {
        return true;
      }
      if (pos > Memory::heap_size) {
        throw std::runtime_error(
            std::format("Attempted to move instruction pointer to "
                        "non-existing address {}",
                        pos.to_uint()));
      }
      instruction_pointer = Number(pos);
      return true;
    }
    case ADD: {
      Word *const aptr = in_prt(memory, instruction_pointer++);
      Word b = out_ptr(memory, instruction_pointer++);
      Word c = out_ptr(memory, instruction_pointer++);
      *aptr = Word((b.to_uint() + c.to_uint()) % 0x8000u) ;
      return true;
    }
    case OUT: {
      Word a = out_ptr(memory, instruction_pointer++);
      assert(a < 256);
      terminal << static_cast<char>(a.to_uint());
      return true;
    }
    case NOOP:
      return true;
  }

  throw std::runtime_error(std::format("Unknown OP code {}", opcode.to_int()));
}

void CPU::Run() noexcept {
  instruction_pointer = Number(0);

  try {
    while (Step()) {
    }
  } catch (std::exception &e) {
    terminal << "\nFATAL ERROR\n" << e.what() << std::endl;
  } catch (...) {
    terminal << "\nFATAL ERROR\nUnknown reasons" << std::endl;
  }
}

}  // namespace SynacorVM