#pragma once

#include <doctest/doctest.h>

#include <fstream>
#include <sstream>

#include "lib/code_generation.hpp"
#include "lib/parser.hpp"
#include "lib/tokenizer.hpp"
#include "testutils/utils.hpp"

inline void test_code_generation(std::string_view test_name,
                                 bool want_success) {
  auto lock = SET_TEST_DIR();

  auto [tokenized, tokenized_ok] = tokenize(fixture_path(test_name));
  REQUIRE_MESSAGE(tokenized_ok, "Setup: unsuccessful tokenization");

  auto [root, parsed_ok] = parse(tokenized.begin(), tokenized.end());
  REQUIRE_MESSAGE(parsed_ok, "Setup: unsuccessful parsing");

  auto [bytecode, success] = generate(root);
  if (want_success) {
    REQUIRE(success);
  } else {
    REQUIRE_FALSE(success);
    return;
  }

  check_golden_binary(test_name, bytecode);
}

TEST_CASE("code_generation") {
  SUBCASE("empty") { test_code_generation("code_generation/empty", true); }
  SUBCASE("endline") { test_code_generation("code_generation/endline", true); }
  SUBCASE("tag") { test_code_generation("code_generation/tag", true); }
  SUBCASE("instruction") {
    test_code_generation("code_generation/instruction", true);
  }
  SUBCASE("numbers") { test_code_generation("code_generation/numbers", true); }
  SUBCASE("sample") { test_code_generation("code_generation/sample", true); }
  SUBCASE("sample2") { test_code_generation("code_generation/sample2", true); }
  SUBCASE("undefined_ref") {
    test_code_generation("code_generation/undefined_ref", false);
  }
}