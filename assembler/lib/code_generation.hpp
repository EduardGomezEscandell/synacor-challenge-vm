#include "grammar.hpp"
#include "parser.hpp"
#include <memory>
#include <ostream>

using bytestr = std::basic_string<std::byte>;

std::pair<bytestr, bool> generate(Node const& root);