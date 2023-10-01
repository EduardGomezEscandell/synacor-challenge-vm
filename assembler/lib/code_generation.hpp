#include <memory>
#include <ostream>

#include "grammar.hpp"
#include "parser.hpp"

using bytestr = std::basic_string<std::byte>;

std::pair<bytestr, bool> generate(Node const& root);