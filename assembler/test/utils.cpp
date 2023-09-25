#include "utils.hpp"

#include <algorithm>
#include <cctype>
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

void reset_path_deleter(std::filesystem::path* p) {
  if (p == nullptr) return;
  std::filesystem::current_path(*p);
  delete (p);
}

std::unique_ptr<std::filesystem::path, void(*)(std::filesystem::path*)> set_test_dir(std::string_view file) {
  auto oldpath = new (std::filesystem::path);
  *oldpath = std::filesystem::current_path();
  auto raii = std::unique_ptr<std::filesystem::path, void(*)(std::filesystem::path*)>(
      oldpath, &reset_path_deleter);

  std::filesystem::current_path(std::filesystem::path(file).parent_path());

  return raii;
}

std::ifstream load_fixture(std::string_view test_name) {
  const auto fixture =
      std::format("testdata/fixtures/{}.in", sanitize(test_name));
  auto f = std::ifstream(fixture.c_str());
  REQUIRE_MESSAGE(
      f, std::format("Setup: could not load fixture file {}", fixture));
  return f;
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
    REQUIRE(true);  // Just so doctest we counts the check
    return;
  }

  char const* const env = std::getenv("GOLDEN_OVERWRITE");
  if (env != nullptr) {
    const auto s = trim(env);
    if (s != "") {
      std::ofstream o(goldenPath.c_str());
      o << got;
      o.close();
    }
  }

  REQUIRE_EQ(want, got);
}