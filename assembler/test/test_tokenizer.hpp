#pragma once

#include <doctest/doctest.h>

#include <sstream>
#include <fstream>

#include "lib/tokenizer.hpp"
#include "test/utils.hpp"

inline void test_tokenizer(std::string_view test_name) {
  auto lock = SET_TEST_DIR();

  const auto got = tokenize(fixture_path(test_name));
  std::stringstream ss;
  fmt_tokens(ss, got);

  check_golden(test_name, ss.str());
}

TEST_CASE("tokenizer") {
  SUBCASE("empty") { test_tokenizer("tokenizer/empty"); }
  SUBCASE("endline") { test_tokenizer("tokenizer/endline"); }
  SUBCASE("tag") { test_tokenizer("tokenizer/tag"); }
  SUBCASE("instruction") { test_tokenizer("tokenizer/instruction"); }
  SUBCASE("numbers") { test_tokenizer("tokenizer/numbers"); }
  SUBCASE("sample") { test_tokenizer("tokenizer/sample"); }
  SUBCASE("sample2") { test_tokenizer("tokenizer/sample2"); }
}