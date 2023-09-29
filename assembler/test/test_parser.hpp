#pragma once

#include <doctest/doctest.h>

#include <fstream>
#include <sstream>

#include "lib/parser.hpp"
#include "lib/tokenizer.hpp"
#include "testutils/utils.hpp"

inline void test_parser(std::string_view test_name, bool want_success) {
  auto lock = SET_TEST_DIR();

  auto [tokenized, ok] = tokenize(testutils::fixture_path(test_name));
  REQUIRE_MESSAGE(ok, "Setup: unsuccessful tokenization");

  auto [root, success] = parse(tokenized.begin(), tokenized.end());
  if (want_success) {
    REQUIRE(success);
  } else {
    REQUIRE_FALSE(success);
  }

  std::stringstream ss;
  ss << root;

  testutils::check_golden(test_name, ss.str());
}

TEST_CASE("parser") {
  SUBCASE("empty") { test_parser("parser/empty", true); }
  SUBCASE("endline") { test_parser("parser/endline", true); }
  SUBCASE("tag") { test_parser("parser/tag", true); }
  SUBCASE("instruction") { test_parser("parser/instruction", true); }
  SUBCASE("numbers") { test_parser("parser/numbers", true); }
  SUBCASE("sample") { test_parser("parser/sample", false); }
  SUBCASE("sample2") { test_parser("parser/sample2", true); }
}