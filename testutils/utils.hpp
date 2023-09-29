#pragma once

#include <filesystem>
#include <memory>
#include <string>

#include "doctest/doctest.h"

#define SET_TEST_DIR() testutils::set_test_dir(__FILE__)
namespace testutils {

std::string sanitize(std::string_view test_name);
std::string_view trim(std::string_view s);

std::unique_ptr<std::filesystem::path, void (*)(std::filesystem::path *)>
set_test_dir(std::string_view file);

std::string fixture_path(std::string_view test_name);

void check_golden(std::string_view test_name, std::string_view got);

void check_golden_binary(std::string_view test_name,
                         std::basic_string_view<std::byte> got);

std::basic_string<std::byte> read_binary(std::string file_name);

} // namespace testutils