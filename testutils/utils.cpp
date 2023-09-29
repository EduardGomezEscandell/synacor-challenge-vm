#include "utils.hpp"

#include <algorithm>
#include <bits/types/FILE.h>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace testutils {

std::string sanitize(std::string_view test_name) {
  std::string sanitized(test_name.size(), 'x');

  std::transform(test_name.cbegin(), test_name.cend(), sanitized.begin(),
                 [](char ch) -> char {
                   switch (ch) {
                   case '\\':
                     return '_';
                   case ' ':
                     return '_';
                   default:
                     return static_cast<char>(std::tolower(ch));
                   }
                 });
  return sanitized;
}

std::string_view trim(std::string_view s) {
  const auto begin = std::find_if_not(
      s.begin(), s.end(), [](char ch) -> bool { return std::isspace(ch); });
  const auto end = std::find_if_not(s.rbegin(), s.rend(), [](char ch) -> bool {
                     return std::isspace(ch);
                   }).base();

  return std::string_view{begin, end};
}

void reset_path_deleter(std::filesystem::path *p) {
  if (p == nullptr)
    return;
  std::filesystem::current_path(*p);
  delete (p);
}

std::unique_ptr<std::filesystem::path, void (*)(std::filesystem::path *)>
set_test_dir(std::string_view file) {
  auto oldpath = new (std::filesystem::path);
  *oldpath = std::filesystem::current_path();
  auto raii =
      std::unique_ptr<std::filesystem::path, void (*)(std::filesystem::path *)>(
          oldpath, &reset_path_deleter);

  const std::filesystem::path dest = std::filesystem::path(file).parent_path();
  INFO(std::format("Running test with CWD located at {}\n", dest.string()));
  std::filesystem::current_path(dest);

  return raii;
}

std::string fixture_path(std::string_view test_name) {
  return std::format("testdata/fixtures/{}.in", sanitize(test_name));
}

void check_golden(std::string_view test_name, std::string_view got) {
  const std::string goldenPath =
      std::format("testdata/golden/{}.out", sanitize(test_name));

  std::ifstream f(goldenPath.c_str());

  REQUIRE_MESSAGE(
      f, std::format("Setup: could not load golden file {}", goldenPath));

  std::stringstream buffer;
  buffer << f.rdbuf();
  const std::string want = buffer.str();

  if (want == got) {
    REQUIRE(true); // Just so doctest counts the check
    return;
  }

  char const *const env = std::getenv("GOLDEN_OVERWRITE");
  if (env != nullptr) {
    const auto s = trim(env);
    if (s != "") {
      std::ofstream o(goldenPath.c_str());
      o << got;
      o.close();
    }
  }

  CHECK_EQ(want, got);
}

void check_golden_binary(std::string_view test_name,
                         std::basic_string_view<std::byte> got) {
  const auto want =
      read_binary(std::format("testdata/golden/{}.out", sanitize(test_name)));

  // Check diff
  auto [iWant, iGot] =
      std::mismatch(want.begin(), want.end(), got.begin(), got.end());
  if (iWant == want.end() && iGot == got.end()) {
    REQUIRE(true); // Just so doctest counts the check
    return;
  }

  // Print diff
  auto idx = std::distance(want.begin(), iWant);
  std::stringstream msg;
  msg << '\n';

  constexpr auto line_len = 32;
  std::size_t line = std::size_t(idx) / line_len;

  // Print header
  msg << "Addr: ";
  for (auto i = 0u; i < line_len / 2; ++i) {
    msg << std::format("{:04x}  ", (line_len * line)/2 + i);
  }
  msg << "\n";

  // Hexdump
  auto write_hex = [&](auto str, std::string_view title) {
    const auto start = line * line_len;
    const auto finish = std::min(start + line_len, str.size());
    msg << std::format("{}: ", title);
    for (auto i = start; i < finish; ++i) {
      msg << std::format("{:02x} ", unsigned(str[i]));
    }
    msg << std::format("\n");
  };

  write_hex(want, "want");
  write_hex(got, " got");

  // Print cursor
  msg << "      ";
  for (auto i = line * line_len; i < std::size_t(idx); ++i) {
    msg << "   ";
  }
  msg << "^diff";

  FAIL(msg.str());
}

std::basic_string<std::byte> read_binary(std::string file_name) {
  // Read binary file
  FILE *f = ::fopen(file_name.c_str(), "rb");
  if (f == nullptr) {
    FAIL(std::format("Setup: could not open binary file {}",
                     (std::filesystem::current_path() / file_name).string()));
  }

  ::fseek(f, 0, SEEK_END);
  auto fsize = std::size_t(::ftell(f));
  ::fseek(f, 0, SEEK_SET);

  std::basic_string<std::byte> buffer(fsize, std::byte(0));
  std::ignore = ::fread(buffer.data(), fsize, 1, f);
  ::fclose(f);

  return buffer;
}

} // namespace testutils