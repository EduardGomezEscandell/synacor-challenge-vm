#pragma once

#include <format>
#include <sstream>
#include <stdexcept>
#include <string>

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

