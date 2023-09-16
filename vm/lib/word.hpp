#pragma once

#include <cassert>
#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace SynacorVM {

template <unsigned MaxBit>
class Value {
  static_assert(MaxBit <= 16);
  std::byte m_data[2];

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

  explicit constexpr Value(std::byte b[2]) {
    m_data[0] = b[0];
    m_data[1] = b[1];

    assert(to_int() < max);
  }

  explicit constexpr Value(std::integral auto x) {
    assert(x >= 0);
    assert(x < max);

    m_data[0] = std::byte(x & 0x00ff);
    m_data[1] = std::byte(x & 0xff00);
  }

  constexpr std::int32_t to_int() const noexcept {
    return (static_cast<std::int32_t>(m_data[1]) << 8) |
           static_cast<std::int32_t>(m_data[0]);
  }

  constexpr std::uint32_t to_uint() const noexcept {
    return (static_cast<std::uint32_t>(m_data[1]) << 8) |
           static_cast<std::uint32_t>(m_data[0]);
  }

  constexpr std::strong_ordering operator<=>(Value other) const noexcept {
    return this->to_int() <=> other.to_int();
  }

  constexpr std::strong_ordering operator<=>(
      std::integral auto other) const noexcept {
    return static_cast<long long>(this->to_int()) <=>
           static_cast<long long>(other);
  }

  Value<15> operator++(int) noexcept {
    const auto ret = Value<15>(m_data);
    *this = Value((this->to_uint() + 1) % modulo);
    return ret;
  }

  constexpr Value<15> operator+(Value const& other) const noexcept {
    const auto sum = this->to_uint() + other.to_uint();
    return Value<15>(sum % static_cast<int>(modulo));
  }
};

using Number = Value<15>;
using Word = Value<16>;

}  // namespace SynacorVM