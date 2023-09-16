#pragma once

#include <cstdint>

inline constexpr auto HALT = std::int32_t(0u);
inline constexpr auto SET = std::int32_t(1u);
inline constexpr auto PUSH = std::int32_t(2u);
inline constexpr auto POP = std::int32_t(3u);
inline constexpr auto EQ = std::int32_t(4u);
inline constexpr auto GT = std::int32_t(5u);
inline constexpr auto JMP = std::int32_t(6u);
inline constexpr auto JT = std::int32_t(7u);
inline constexpr auto JF = std::int32_t(8u);
inline constexpr auto ADD = std::int32_t(9u);
inline constexpr auto MULT = std::int32_t(10u);
inline constexpr auto MOD = std::int32_t(11u);
inline constexpr auto AND = std::int32_t(12u);
inline constexpr auto OR = std::int32_t(13u);
inline constexpr auto NOT = std::int32_t(14u);
inline constexpr auto RMEM = std::int32_t(15u);
inline constexpr auto WMEM = std::int32_t(16u);
inline constexpr auto CALL = std::int32_t(17u);
inline constexpr auto RET = std::int32_t(18u);
inline constexpr auto OUT = std::int32_t(19u);
inline constexpr auto IN = std::int32_t(20u);
inline constexpr auto NOOP = std::int32_t(21u);