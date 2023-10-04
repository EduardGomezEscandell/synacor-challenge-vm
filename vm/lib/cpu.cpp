#include "cpu.hpp"

#include <exception>
#include <format>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "arch/arch.hpp"
#include "vm/lib/memory.hpp"
#include "vm/lib/word.hpp"

namespace SynacorVM {

Word &get_register(Memory &m, Number ptr) {
  const Word v = m[ptr];

  int p = static_cast<int>(v.to_uint()) - static_cast<int>(Memory::heap_size);
  if (p < 0 || unsigned(p) > Memory::register_count) {
    throw std::runtime_error(std::format(
        "Attempted to access non-existing register with code {:04x}",
        v.to_uint()));
  }

  return m[v];
}

Word value_or_register(Memory &m, Number ptr) {
  const Word v = m[ptr];
  if (v < Memory::heap_size) {
    // Number literal
    return v;
  }

  // Register
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
    Word &regA = get_register(memory, instruction_pointer++);
    Word const b = value_or_register(memory, instruction_pointer++);
    regA = b;
    return true;
  }
  case PUSH: {
    const Word w = value_or_register(memory, instruction_pointer++);
    memory.push(w);
    return true;
  }
  case POP: {
    Word &regA = get_register(memory, instruction_pointer++);
    if (memory.stack_ptr() == 0) {
      throw std::runtime_error("Called POP with an empty stack");
    }
    regA = memory.pop();
    return true;
  }
  case EQ: {
    Word &regA = get_register(memory, instruction_pointer++);
    Word const b = value_or_register(memory, instruction_pointer++);
    Word const c = value_or_register(memory, instruction_pointer++);
    regA = (b == c) ? Word(1) : Word(0);
    return true;
  }
  case GT: {
    Word &regA = get_register(memory, instruction_pointer++);
    Word const b = value_or_register(memory, instruction_pointer++);
    Word const c = value_or_register(memory, instruction_pointer++);
    regA = (b > c) ? Word(1) : Word(0);
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
    Word &regA = get_register(memory, instruction_pointer++);
    Word const b = value_or_register(memory, instruction_pointer++);
    Word const c = value_or_register(memory, instruction_pointer++);
    regA = Word((b.to_uint() + c.to_uint()) % 0x8000u);
    return true;
  }
  case MULT: {
    Word &regA = get_register(memory, instruction_pointer++);
    Word const b = value_or_register(memory, instruction_pointer++);
    Word const c = value_or_register(memory, instruction_pointer++);
    regA = Word((b.to_uint() * c.to_uint()) % 0x8000u);
    return true;
  }
  case MOD: {
    Word &regA = get_register(memory, instruction_pointer++);
    Word const b = value_or_register(memory, instruction_pointer++);
    Word const c = value_or_register(memory, instruction_pointer++);
    regA = Word(b.to_uint() % c.to_uint());
    return true;
  }
  case AND: {
    Word &regA = get_register(memory, instruction_pointer++);
    Word const b = value_or_register(memory, instruction_pointer++);
    Word const c = value_or_register(memory, instruction_pointer++);
    regA = b & c;
    return true;
  }
  case OR: {
    Word &regA = get_register(memory, instruction_pointer++);
    Word const b = value_or_register(memory, instruction_pointer++);
    Word const c = value_or_register(memory, instruction_pointer++);
    regA = b | c;
    return true;
  }
  case NOT: {
    Word &regA = get_register(memory, instruction_pointer++);
    Word const b = value_or_register(memory, instruction_pointer++);
    regA = ~b;
    return true;
  }
  case RMEM: {
    Word &regA = get_register(memory, instruction_pointer++);
    Word const ptr = value_or_register(memory, instruction_pointer++);
    regA = memory[ptr];
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
    if (memory.stack_ptr() == 0) {
      return false;
    }
    Word const pos = memory.pop();
    instruction_pointer = jump(pos);
    return true;
  }
  case OUT: {
    Word const a = value_or_register(memory, instruction_pointer++);
    assert(a < 256);
    const auto ch = static_cast<char>(a.to_uint());
    if(ch == '\n') {
      *stdOut << std::endl;
    } else {
      *stdOut << ch;
    }
    return true;
  }
  case IN: {
    Word &regA = get_register(memory, instruction_pointer++);
    auto w = stdIn->get();
    if(w == std::char_traits<char>::eof()) {
      throw std::runtime_error("could not read from stdin");
    }
    regA = Word(w);
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
    bool keep_running = true;
    while (keep_running) {
      if(pre_exec_hook != nullptr) {
        pre_exec_hook(execution_state(*this));
      }
      
      keep_running = Step();

      if(post_exec_hook != nullptr) {
        post_exec_hook(execution_state(*this), keep_running);
      }
    }
  } catch (std::exception &e) {
    *stdOut << "\nFATAL ERROR\n" << e.what() << std::endl;
  } catch (...) {
    *stdOut << "\nFATAL ERROR\nUnknown reasons" << std::endl;
  }
}

} // namespace SynacorVM