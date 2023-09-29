#pragma once

#include <doctest/doctest.h>

#include <sstream>
#include <fstream>

#include "lib/tokenizer.hpp"
#include "testutils/utils.hpp"

inline void test_tokenizer(std::string_view test_name, bool want_success) {
  auto lock = SET_TEST_DIR();

  const auto [got, ok] = tokenize(testutils::fixture_path(test_name));
  if(want_success) {
    REQUIRE(ok);
  } else {
    REQUIRE_FALSE(ok);
  }

  std::stringstream ss;
  fmt_tokens(ss, got);

  testutils::check_golden(test_name, ss.str());
}

TEST_CASE("tokenizer") {
  SUBCASE("empty") { test_tokenizer("tokenizer/empty", true); }
  SUBCASE("endline") { test_tokenizer("tokenizer/endline", true); }
  SUBCASE("tag") { test_tokenizer("tokenizer/tag", true); }
  SUBCASE("string") { test_tokenizer("tokenizer/string", true); }
  SUBCASE("char") { test_tokenizer("tokenizer/char", true); }
  SUBCASE("instruction") { test_tokenizer("tokenizer/instruction", true); }
  SUBCASE("numbers") { test_tokenizer("tokenizer/numbers", false); }
  SUBCASE("sample") { test_tokenizer("tokenizer/sample", true); }
  SUBCASE("sample2") { test_tokenizer("tokenizer/sample2", true); }
}