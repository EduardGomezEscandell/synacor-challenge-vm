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

#define CPU_SUBCASE(name)  SUBCASE(name) { test_cpu(std::format("cpu/{}", name)); }

TEST_CASE("cpu") {
  CPU_SUBCASE("halt")
  CPU_SUBCASE("out")
  CPU_SUBCASE("set")
  CPU_SUBCASE("push-pop-register")
  CPU_SUBCASE("push-pop-memory")
  CPU_SUBCASE("eq")
  CPU_SUBCASE("gt")
  CPU_SUBCASE("jmp")
  CPU_SUBCASE("jt")
  CPU_SUBCASE("jf")
  CPU_SUBCASE("add")
  CPU_SUBCASE("mult")
  CPU_SUBCASE("mod")
  CPU_SUBCASE("and")
  CPU_SUBCASE("or")
  CPU_SUBCASE("not")
}