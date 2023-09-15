#include <algorithm>
#include <concepts>
#include <istream>
#include <iterator>
#include <stdexcept>
#include <vector>
#include <array>

#include "word.hpp"

namespace SynacorVM {

class Memory {
 public:
  constexpr static unsigned register_count = 8;
  constexpr static unsigned heap_size = 1 << 15;

 private:
  std::array<Word, register_count> m_registers;
  std::array<Word, heap_size> m_heap;

  std::vector<Word> m_stack;

 public:
  Word& operator[](Word idx)  {
    auto i = idx.to_uint();
    if (i < heap_size) {
      return m_heap[i];
    }

    i -= heap_size;
    if (i < register_count) {
      return m_registers[i];
    }

    throw std::runtime_error("Invalid idx");
  }

  Word operator[](Word idx) const{
    return const_cast<Memory*>(this)->operator[](idx);
  };

  Word operator[](Number addr) const noexcept { return m_heap[addr.to_uint()]; }

  Word& operator[](Number addr) noexcept { return m_heap[addr.to_uint()]; }

  std::size_t Load(std::istream& in) {
    std::byte n[2];
    unsigned i;
    for (i = 0; in && i < heap_size; ++i) {
      in.read(reinterpret_cast<char*>(n), 2);
      m_heap[i] = Word(n);
    }

    return i;
  }
};

}  // namespace SynacorVM