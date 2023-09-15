#include "cpu.hpp"

#include <exception>
#include <istream>
#include <ostream>
#include <string_view>

namespace SynacorVM {

Word* in_prt(Memory& m, Number ptr) {
  const Word v = m[ptr];
  return &m[v];
}

Word out_ptr(Memory& m, Number ptr) {
  const Word v = m[ptr];
  if (v < Memory::heap_size) {
    return v;
  }

  return m[v];
}

void CPU::Run() noexcept {
  instruction_pointer = Number(0);

  try {
    while (true) {
      auto opcode = Number(mem[instruction_pointer++].to_uint());
      switch (opcode.to_int()) {
        case HALT:
          return;
        case SET: {
          Word* aptr = in_prt(mem, instruction_pointer++);
          Word b = out_ptr(mem, instruction_pointer++);
          *aptr = b;
          return;
        }
        case ADD: {
          Word* aptr = in_prt(mem, instruction_pointer++);
          Word b = out_ptr(mem, instruction_pointer++);
          Word c = out_ptr(mem, instruction_pointer++);
          *aptr = Word(b + c);
          break;
        }
        case OUT: {
          Word a = out_ptr(mem, instruction_pointer++);
          assert(a < 256);
          terminal << static_cast<char>(a.to_uint());
          break;
        }
        case NOOP:
          break;
      }
    }
  } catch (std::exception& e) {
    terminal << "\nFATAL ERROR\n" << e.what() << std::endl;
  } catch (...) {
    terminal << "\nFATAL ERROR\nUnknown reasons" << std::endl;
  }
}

}  // namespace SynacorVM