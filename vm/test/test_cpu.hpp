#pragma once

#include "lib/cpu.hpp"
#include "lib/memory.hpp"
#include "testutils/utils.hpp"
#include <cstdio>
#include <doctest/doctest.h>
#include <format>
#include <sstream>

inline void test_cpu(std::string_view test_name) {
  auto lock = SET_TEST_DIR();

  std::stringstream out;
  SynacorVM::Memory ram;

  SynacorVM::CPU vm{.memory = ram, .terminal = out};

  const auto buff = testutils::read_binary(testutils::fixture_path(test_name));
  ram.load(buff);

  vm.Run();

  testutils::check_golden_binary(std::format("{}/memory", test_name),
                                 ram.dump());
  testutils::check_golden(std::format("{}/stdout", test_name), out.str());
}

TEST_CASE("cpu") {
  SUBCASE("halt") { test_cpu("cpu/halt"); }
  SUBCASE("out") { test_cpu("cpu/out"); }
  SUBCASE("set") { test_cpu("cpu/set"); }
  SUBCASE("add") { test_cpu("cpu/add"); }
  SUBCASE("push-pop-register") { test_cpu("cpu/push-pop-register"); }
  SUBCASE("push-pop-memory") { test_cpu("cpu/push-pop-memory"); }
}