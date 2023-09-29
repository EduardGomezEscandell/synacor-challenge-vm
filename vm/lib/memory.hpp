#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <istream>
#include <iterator>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

#include "word.hpp"

namespace SynacorVM {

class Memory {
public:
  constexpr static unsigned register_count = 8;
  constexpr static unsigned heap_size = 1 << 15;

private:
  std::array<Word, register_count> m_registers;
  std::array<Word, heap_size> m_heap;
  std::stack<Word> m_stack;

public:
  Word &operator[](Word idx) {
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

  Word operator[](Word idx) const {
    return const_cast<Memory *>(this)->operator[](idx);
  };

  Word operator[](Number addr) const noexcept { return m_heap[addr.to_uint()]; }

  Word &operator[](Number addr) noexcept { return m_heap[addr.to_uint()]; }

  void push(Word val) {
    m_stack.push(val);
  }

  Word pop() {
    Word n = m_stack.top();
    m_stack.pop();
    return n;
  }

  void load(std::basic_string<std::byte> in) {
    std::size_t len = in.size();
    if (len > heap_size*2) {
      len = heap_size*2;
    }

    ::memset((std::byte*)&m_heap[0], 0, 2*heap_size);
    ::memcpy((std::byte*)&m_heap[0], &in[0], len);
  }

  std::basic_string<std::byte> dump() const {
    auto last = std::find_if_not(m_heap.crbegin(), m_heap.crend(),
                     [](Word const word) -> bool { return word.to_int() == 0; });

    const auto size = 2 * std::size_t(std::distance(last, m_heap.crend()));
    std::basic_string<std::byte> out(size, std::byte(0));

    ::memcpy(&out[0], &m_heap[0], size);
    return out;
  }
};

} // namespace SynacorVM