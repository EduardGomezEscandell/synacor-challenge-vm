#pragma once

#include <concepts>
#include <format>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "assembler/lib/grammar.hpp"
#include "lib/cpu.hpp"
#include "lib/memory.hpp"

inline std::basic_string<std::byte> read_binary(std::string file_name) {
  // Read binary file
  FILE *f = ::fopen(file_name.c_str(), "rb");
  if (f == nullptr) {
    throw std::runtime_error(
        std::format("Setup: could not read binary file {}", file_name));
  }

  ::fseek(f, 0, SEEK_END);
  auto fsize = std::size_t(::ftell(f));
  ::fseek(f, 0, SEEK_SET);

  std::basic_string<std::byte> buffer(fsize, std::byte(0));
  std::ignore = ::fread(buffer.data(), fsize, 1, f);
  ::fclose(f);

  return buffer;
}

template <typename T> struct deferrer {
  T callable;
  ~deferrer() { callable(); }

  deferrer(T &&t) : callable(std::forward<T &&>(t)) {}

  deferrer(deferrer const &other) = delete;

  deferrer(deferrer &&other) { *this = other; }

  deferrer &operator=(deferrer &&other) {
    if (this == &other) {
      return *this;
    }
    std::swap(callable, other.callable);
    return *this;
  }
};

// defer calls the lambda when the returned object is destroyed.
template <typename F> [[nodiscard]] constexpr auto defer(F &&f) {
  return deferrer<F>(std::forward<F &&>(f));
}