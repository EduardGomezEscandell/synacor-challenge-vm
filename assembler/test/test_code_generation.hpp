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

  auto [tokenized, tokenized_ok] = tokenize(testutils::fixture_path(test_name));
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

  testutils::check_golden_binary(test_name, bytecode);
}

#define SUBCASE_CG(name, wantSuccess) \
  SUBCASE(name) { test_code_generation("code_generation/" name, wantSuccess); }

TEST_CASE("code_generation") {
  SUBCASE_CG("empty", true)
  SUBCASE_CG("endline", true)
  SUBCASE_CG("tag", true)
  SUBCASE_CG("instruction", true)
  SUBCASE_CG("numbers", true)
  SUBCASE_CG("sample", true)
  SUBCASE_CG("sample2", true)
  SUBCASE_CG("undefined_ref", false);
}