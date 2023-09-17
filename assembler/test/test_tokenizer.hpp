#pragma once

#include <doctest/doctest.h>

#include <sstream>

#include "lib/tokenizer.hpp"
#include "test/utils.hpp"

namespace {
inline void test(std::string_view test_name) {
  auto lock = SET_TEST_DIR();

  std::ifstream input = load_fixture(test_name);

  const auto got = tokenize(input);
  std::stringstream ss;
  fmt_tokens(ss, got);

  check_golden(test_name, ss.str());
}
}  // namespace

TEST_CASE("empty") { test("empty"); }
TEST_CASE("endline") { test("endline"); }
TEST_CASE("tag") { test("tag"); }
TEST_CASE("instruction") { test("instruction"); }
TEST_CASE("numbers") { test("numbers"); }
TEST_CASE("sample") { test("sample"); }
TEST_CASE("sample2") { test("sample2"); }