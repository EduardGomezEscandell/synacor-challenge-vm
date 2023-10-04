#pragma once

#include "helpers.hpp"
#include "lib/memory.hpp"

#include <concepts>
#include <map>
#include <set>
#include <stdexcept>
#include <string>

std::string parse_value(SynacorVM::Word w);
std::string peek_instruction(SynacorVM::execution_state es);

struct coverage {
  std::array<bool, SynacorVM::Memory::heap_size> visited{};

  [[nodiscard]] auto pre_exec_hook() {
    return [this](SynacorVM::execution_state es) { this->update_coverage(es); };
  }

  [[nodiscard]] std::string summary() {
    auto visit_count = std::count(visited.begin(), visited.end(), 0);
    std::stringstream ss;
    ss << "\n-------------\n"
       << std::format("Covered {} addresses ({:.2} %)", visit_count,
                      float(visit_count) / float(0x8000));
    ss << "\n-------------\n";
    return ss.str();
  }

private:
  void update_coverage(SynacorVM::execution_state es) {
    const SynacorVM::Word verb = es.heap[es.instruction_ptr.to_uint()];
    const auto v = static_cast<Verb>(verb.to_uint());
    const auto argc = arch::argument_count(v);
    if (argc < 0) {
      std::cerr << std::format("UNKNOWN {:x}\n", verb.to_uint());
      return;
    }

    const auto begin = visited.begin() + es.instruction_ptr.to_uint();
    const auto end = begin + 1 + argc;
    std::fill(begin, end, true);
  }
};

struct command_preprocessor {
  command_preprocessor(std::istream &in) : in(in) {}

  void install(SynacorVM::CPU &target);

  void toggle_hook(std::string const &name,
                   std::function<void(SynacorVM::execution_state)> &&f);

  template <typename T> void enqueue(T s) {
    queued_chars += s.size();
    out << s;
  }

  template <std::convertible_to<char> T> void enqueue(T s) {
    out << s;
    ++queued_chars;
  }

  void set_sleep(long x) { sleep = x; }

  bool toggle_dbg_point(unsigned long x) {
    if (x > SynacorVM::Memory::heap_size) {
      throw std::runtime_error("cannot set debug point outside memory range");
    }

    const auto u = unsigned(x);
    if (debug_points.contains(u)) {
      debug_points.erase(u);
      return false;
    }
    debug_points.emplace(u);
    return true;
  }

private:
  SynacorVM::CPU *cpu;

  std::map<std::string, std::function<void(SynacorVM::execution_state)>>
      other_prehooks;

  std::istream &in;
  std::stringstream out;
  std::size_t queued_chars = 0;

  bool first_instruction = true;

  long sleep = 0;
  std::set<unsigned> debug_points;

  bool command(std::string cmd, SynacorVM::execution_state es);
  void pre_exec_hook(SynacorVM::execution_state es);
};