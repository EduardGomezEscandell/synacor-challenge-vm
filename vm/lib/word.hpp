#pragma once

#include <array>
#include <cassert>
#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace SynacorVM {

template <unsigned MaxBit>
class Value {
  static_assert(MaxBit <= 16);
  std::array<std::byte, 2> m_data;

 public:
  constexpr static std::int32_t max = 1 << MaxBit;
  constexpr static std::int32_t modulo = 1 << 15;

  explicit Value() {}

  template <unsigned M>
  explicit Value(Value<M> const& other) {
    const auto num = other.to_uint();
    assert(num < max);
    *this = Value(num);
  }

  explicit constexpr Value(std::array<std::byte, 2> b) : m_data(b) {
    assert(to_int() < max);
  }

  explicit constexpr Value(std::integral auto x) {
    assert(x >= 0);
    assert(x < max);

    m_data[0] = std::byte(x & 0x00ff);
    m_data[1] = std::byte((x & 0xff00) >> 8);
  }

  constexpr std::int32_t to_int() const noexcept {
    return (static_cast<std::int32_t>(m_data[1]) << 8) |
           static_cast<std::int32_t>(m_data[0]);
  }

  constexpr std::uint32_t to_uint() const noexcept {
    return (static_cast<std::uint32_t>(m_data[1]) << 8) |
           static_cast<std::uint32_t>(m_data[0]);
  }

  constexpr bool nonzero() const noexcept { return to_int() != 0; }

  template <unsigned N>
  constexpr std::strong_ordering operator<=>(Value<N> other) const noexcept {
    return this->to_int() <=> other.to_int();
  }

  template <unsigned N>
  constexpr bool operator==(Value<N> other) const noexcept {
    return this->to_int() == other.to_int();
  }

  template <unsigned N>
  constexpr bool operator!=(Value<N> other) const noexcept {
    return this->to_int() != other.to_int();
  }

  using biggest_int = long long int;

  template <std::integral T>
  constexpr std::strong_ordering operator<=>(T other) const noexcept {
    return biggest_int(this->to_int()) <=> biggest_int(other);
  }

  constexpr bool operator==(std::integral auto other) const noexcept {
    return biggest_int(this->to_int()) != biggest_int(other);
  }

  constexpr bool operator!=(std::integral auto other) const noexcept {
    return biggest_int(this->to_int()) != biggest_int(other);
  }

  Value operator++(int) noexcept {
    const auto ret = Value<15>(m_data);
    *this = Value((this->to_uint() + 1) % modulo);
    return ret;
  }

  friend Value operator&(Value left, Value right) noexcept {
    return Value(std::array{
        left.m_data[0] & right.m_data[0],
        left.m_data[1] & right.m_data[1],
    });
  }

  friend Value operator|(Value left, Value right) noexcept {
    return Value(std::array{
        left.m_data[0] | right.m_data[0],
        left.m_data[1] | right.m_data[1],
    });
  }

  // 15 byte inverse
  Value operator~() const noexcept {
    return Value(std::array{
        ~m_data[0],
        ~m_data[1] & std::byte(0x7fu),
    });
  }
};

using Number = Value<15>;
using Word = Value<16>;

}  // namespace SynacorVM