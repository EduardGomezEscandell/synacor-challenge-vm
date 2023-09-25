#pragma once

#include <doctest/doctest.h>

#include <fstream>
#include <sstream>

#include "lib/parser.hpp"
#include "lib/tokenizer.hpp"
#include "test/utils.hpp"

inline void test_parser(std::string_view test_name, bool want_success) {
  auto lock = SET_TEST_DIR();

  auto tokenized = tokenize(fixture_path(test_name));
  auto [root, success] = parse(tokenized.begin(), tokenized.end());
  if (!want_success) {
    REQUIRE_FALSE(success);
  } else {
    REQUIRE(success);
  }

  std::stringstream ss;
  ss << root;

  check_golden(test_name, ss.str());
}

TEST_CASE("parser") {
  SUBCASE("empty") { test_parser("parser/empty", true); }
  SUBCASE("endline") { test_parser("parser/endline", true); }
  SUBCASE("tag") { test_parser("parser/tag", true); }
  SUBCASE("instruction") { test_parser("parser/instruction", true); }
  SUBCASE("numbers") { test_parser("parser/numbers", false); }
  SUBCASE("sample") { test_parser("parser/sample", false); }
  SUBCASE("sample2") { test_parser("parser/sample2", true); }
}