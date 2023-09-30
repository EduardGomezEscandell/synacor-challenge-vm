#include "cpu.hpp"

#include <exception>
#include <format>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string_view>

#include "arch/arch.hpp"

namespace SynacorVM {

Word *write_prt(Memory &m, Number ptr) {
  const Word v = m[ptr];
  return &m[v];
}

Word value_or_register(Memory &m, Number ptr) {
  const Word v = m[ptr];
  if (v < Memory::heap_size) {
    return v;
  }

  return m[v];
}

[[nodiscard]] Number jump(Word destination) {
  if (destination > Memory::heap_size) {
    throw std::runtime_error(std::format(
        "Attempted to move instruction pointer to inexistent address {:04x}",
        destination.to_uint()));
  }
  return Number(destination);
}

bool CPU::Step() {
  auto opcode = Number(memory[instruction_pointer++].to_uint());
  switch (opcode.to_int()) {
    case HALT:
      return false;
    case SET: {
      Word *const a = write_prt(memory, instruction_pointer++);
      Word const b = value_or_register(memory, instruction_pointer++);
      *a = b;
      return true;
    }
    case PUSH: {
      const Word w = value_or_register(memory, instruction_pointer++);
      memory.push(w);
      return true;
    }
    case POP: {
      Word *const ptr = write_prt(memory, instruction_pointer++);
      if (memory.stack_ptr() == 0) {
        throw std::runtime_error("Called POP with an empty stack");
      }
      *ptr = memory.pop();
      return true;
    }
    case EQ: {
      Word *const a = write_prt(memory, instruction_pointer++);
      Word const b = value_or_register(memory, instruction_pointer++);
      Word const c = value_or_register(memory, instruction_pointer++);
      *a = (b == c) ? Word(1) : Word(0);
      return true;
    }
    case GT: {
      Word *const a = write_prt(memory, instruction_pointer++);
      Word const b = value_or_register(memory, instruction_pointer++);
      Word const c = value_or_register(memory, instruction_pointer++);
      *a = (b > c) ? Word(1) : Word(0);
      return true;
    }
    case JMP: {
      Word const pos = value_or_register(memory, instruction_pointer++);
      instruction_pointer = jump(pos);
      return true;
    }
    case JT: {
      Word cond = value_or_register(memory, instruction_pointer++);
      Word const pos = value_or_register(memory, instruction_pointer++);
      if (!cond.nonzero()) {
        return true;
      }
      instruction_pointer = jump(pos);
      return true;
    }
    case JF: {
      Word cond = value_or_register(memory, instruction_pointer++);
      Word const pos = value_or_register(memory, instruction_pointer++);
      if (cond.nonzero()) {
        return true;
      }
      instruction_pointer = jump(pos);
      return true;
    }
    case ADD: {
      Word *const a = write_prt(memory, instruction_pointer++);
      Word const b = value_or_register(memory, instruction_pointer++);
      Word const c = value_or_register(memory, instruction_pointer++);
      *a = Word((b.to_uint() + c.to_uint()) % 0x8000u);
      return true;
    }
    case MULT: {
      Word *const a = write_prt(memory, instruction_pointer++);
      Word const b = value_or_register(memory, instruction_pointer++);
      Word const c = value_or_register(memory, instruction_pointer++);
      *a = Word((b.to_uint() * c.to_uint()) % 0x8000u);
      return true;
    }
    case MOD: {
      Word *const a = write_prt(memory, instruction_pointer++);
      Word const b = value_or_register(memory, instruction_pointer++);
      Word const c = value_or_register(memory, instruction_pointer++);
      *a = Word(b.to_uint() % c.to_uint());
      return true;
    }
    case AND: {
      Word *const a = write_prt(memory, instruction_pointer++);
      Word const b = value_or_register(memory, instruction_pointer++);
      Word const c = value_or_register(memory, instruction_pointer++);
      *a = b & c;
      return true;
    }
    case OR: {
      Word *const a = write_prt(memory, instruction_pointer++);
      Word const b = value_or_register(memory, instruction_pointer++);
      Word const c = value_or_register(memory, instruction_pointer++);
      *a = b | c;
      return true;
    }
    case NOT: {
      Word *const a = write_prt(memory, instruction_pointer++);
      Word const b = value_or_register(memory, instruction_pointer++);
      *a = ~b;
      return true;
    }
    case RMEM: {
      Word *const dst = write_prt(memory, instruction_pointer++);
      Word const ptr = value_or_register(memory, instruction_pointer++);
      *dst = memory[ptr];
      return true;
    }
    case WMEM: {
      Word const ptr = value_or_register(memory, instruction_pointer++);
      Word const src = value_or_register(memory, instruction_pointer++);
      memory[ptr] = src;
      return true;
    }
    case CALL: {
      Word const pos = value_or_register(memory, instruction_pointer++);
      memory.push(Word(instruction_pointer));
      instruction_pointer = jump(pos);
      return true;
    }
    case RET: {
      if(memory.stack_ptr() == 0) {
        return false;
      }
      Word const pos = memory.pop();
      instruction_pointer = jump(pos);
      return true;
    }
    case OUT: {
      Word a = value_or_register(memory, instruction_pointer++);
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