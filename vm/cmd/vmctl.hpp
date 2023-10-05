#pragma once

#include "arch/arch.hpp"
#include "helpers.hpp"
#include "lib/cpu.hpp"
#include "lib/memory.hpp"

#include <concepts>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>

struct command_preprocessor;

struct cmd {
  std::string name;
  std::string usage;
  std::string help;
  std::function<bool(SynacorVM::execution_state state,
                     std::stringstream &argstream)>
      f;
};

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

inline auto next_word(std::stringstream &ss) -> std::string {
  std::string v;
  ss >> v;
  return v;
};

struct command_preprocessor {
  command_preprocessor(std::istream &in, std::unique_ptr<coverage> &cover)
      : in(in), commands{
                    cmd_setr(*this),   cmd_skipn(*this),  cmd_step(*this),
                    cmd_abreak(*this), cmd_ibreak(*this), cmd_peek(*this),
                    cmd_instr(*this),  cmd_exit(*this),   cmd_cov(*this, cover),
                    cmd_help(*this),   cmd_cont(*this)} {}

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

  bool toggle_addr_breakpoint(unsigned long x) {
    if (x > SynacorVM::Memory::heap_size) {
      throw std::runtime_error("cannot set debug point outside memory range");
    }

    const auto u = unsigned(x);
    if (addr_breakpoints.contains(u)) {
      addr_breakpoints.erase(u);
      return false;
    }
    addr_breakpoints.emplace(u);
    return true;
  }

  bool toggle_instr_breakpoint(Verb v) {
    if (v >= ERROR) {
      throw std::runtime_error(
          "cannot set debug point outside instruction range");
    }

    const auto u = unsigned(v);
    if (instr_breakpoints.contains(u)) {
      instr_breakpoints.erase(u);
      return false;
    }
    instr_breakpoints.emplace(u);
    return true;
  }

private:
  SynacorVM::CPU *cpu;

  std::map<std::string, std::function<void(SynacorVM::execution_state)>>
      other_prehooks;

  std::istream &in;
  std::stringstream out;
  std::size_t queued_chars = 0;

  std::map<std::string, cmd> commands;

  bool first_instruction = true;

  long sleep = 0;
  std::set<unsigned> addr_breakpoints;
  std::set<unsigned> instr_breakpoints;

  bool command(std::string cmd, SynacorVM::execution_state es);
  void pre_exec_hook(SynacorVM::execution_state es);

  static std::pair<std::string, cmd> cmd_setr(command_preprocessor &) {
    cmd command{
        .name = "!setr",
        .usage = "!setr <REG> <VALUE>",
        .help = "sets register REG to VALUE",
        .f = [&](auto es, auto &argstream) -> bool {
          const auto reg = std::stoul(next_word(argstream), nullptr, 0);
          const auto value = std::stoul(next_word(argstream), nullptr, 0);
          es.registers.at(reg) = SynacorVM::Word(value);
          std::cerr << std::format("Set register {} to 0x{:04x}\n", reg, value)
                    << std::flush;
          return false;
        }};
    return {command.name, command};
  }
  static std::pair<std::string, cmd> cmd_skipn(command_preprocessor &p) {
    cmd command{
        .name = "!skip",
        .usage = "!skip <N> ",
        .help = "Advances N instructions and then stops. It may stop earlier "
                "if STDIN input is needed, but it'll stop again in the "
                "specified point.",
        .f = [&](auto, auto &argstream) -> bool {
          const auto s = std::stoll(next_word(argstream), nullptr, 0);
          p.set_sleep(s);
          std::cerr << std::format(
                           "You'll be prompted again in {} instructions "
                           "(Unless input is needed before)\n",
                           s)
                    << std::flush;
          return true;
        }};
    return {command.name, command};
  }
  static std::pair<std::string, cmd> cmd_step(command_preprocessor &p) {
    cmd command{.name = "!step",
                .usage = "!step",
                .help = "Advances one instruction. Equivalent to 'skip 1'",
                .f = [&](auto, auto &) -> bool {
                  p.set_sleep(1);
                  return true;
                }};
    return {command.name, command};
  }
  static std::pair<std::string, cmd> cmd_abreak(command_preprocessor &p) {
    cmd command{.name = "!abreak",
                .usage = "!abreak <ADDRESS>",
                .help = "Toggles a breakpoint at the specified address.",
                .f = [&](auto, auto &argstream) -> bool {
                  const auto s = std::stoul(next_word(argstream), nullptr, 0);
                  if (p.toggle_addr_breakpoint(s)) {
                    std::cerr << std::format("Added breakpoint at {:04x}\n", s)
                              << std::flush;
                  } else {
                    std::cerr
                        << std::format("Removed breakpoint at {:04x}\n", s)
                        << std::flush;
                  }
                  return false;
                }};
    return {command.name, command};
  }

  static std::pair<std::string, cmd> cmd_ibreak(command_preprocessor &p) {
    cmd command{.name = "!ibreak",
                .usage = "!ibreak <INSTR>",
                .help = "Toggles a breakpoint at the specified instruction",
                .f = [&](auto, auto &ss) -> bool {
                  const Verb v = arch::from_string(next_word(ss));
                  if (v == ERROR) {
                    throw std::runtime_error("Invalid instruction name");
                  }

                  if (p.toggle_instr_breakpoint(v)) {
                    std::cerr << std::format("Added breakpoint for {}\n",
                                             arch::to_string(v))
                              << std::flush;
                  } else {
                    std::cerr << std::format("Removed breakpoint for {}\n",
                                             arch::to_string(v))
                              << std::flush;
                  }
                  return false;
                }};
    return {command.name, command};
  }

  static std::pair<std::string, cmd> cmd_peek(command_preprocessor &) {
    cmd command{.name = "!peek",
                .usage = "!peek",
                .help = "Shows the next instruction to execute. It also "
                        "displays the registers",
                .f = [](auto es, auto &) -> bool {
                  std::cerr << peek_instruction(es) << std::flush;
                  return false;
                }};
    return {command.name, command};
  }
  static std::pair<std::string, cmd> cmd_instr(command_preprocessor &p) {
    cmd command{.name = "!instr",
                .usage = "!instr",
                .help = "Toggles instruction logging",
                .f = [&](auto, auto &) -> bool {
                  p.toggle_hook("instruction printing", [](auto es) {
                    std::cerr << peek_instruction(es) << std::flush;
                  });
                  return false;
                }};
    return {command.name, command};
  }
  static std::pair<std::string, cmd> cmd_exit(command_preprocessor &p) {
    cmd command{.name = "!exit",
                .usage = "!exit",
                .help = "Stops the machine by overwriting a HALT at the "
                        "current position pointed by the instruction pointer",
                .f = [&](auto es, auto &) -> bool {
                  es.heap[es.instruction_ptr.to_uint()] =
                      SynacorVM::Word(static_cast<unsigned>(Verb::HALT));
                  std::cerr << "Exiting\n" << std::flush;
                  p.enqueue(std::char_traits<char>::eof());
                  return true;
                }};
    return {command.name, command};
  }
  static std::pair<std::string, cmd> cmd_cov(command_preprocessor &p,
                                             std::unique_ptr<coverage> &cov) {
    cmd command{.name = "!cov",
                .usage = "!cov",
                .help = "Toggle coverage profiling",
                .f = [&](auto, auto &) -> bool {
                  if (cov.get() == nullptr) {
                    cov = std::make_unique<coverage>();
                  }
                  p.toggle_hook("compute coverage", cov->pre_exec_hook());
                  return false;
                }};
    return {command.name, command};
  }
  static std::pair<std::string, cmd> cmd_help(command_preprocessor &p) {
    cmd command{.name = "!help",
                .usage = "!help",
                .help = "Prints this message",
                .f = [&](auto, auto &) -> bool {
                  std::stringstream ss;
                  ss << "Use any of these commads.\n"
                     << "You can escape the leading ! by writting !!\n"
                     << std::flush;
                  ss << "---------------------+--------------------------------"
                        "---------------------\n";
                  ss << std::format("{: <20} | {}\n", "Usage", "Help");
                  ss << "---------------------+--------------------------------"
                        "---------------------\n";
                  for (auto const &[key, cmd] : p.commands) {
                    ss << std::format("{: <20} | {}\n", cmd.usage, cmd.help);
                  }
                  ss << "---------------------+--------------------------------"
                        "---------------------\n";
                  std::cerr << ss.str() << std::flush;
                  return false;
                }};
    return {command.name, command};
  }

  static std::pair<std::string, cmd> cmd_cont(command_preprocessor &) {
    cmd command{
        .name = "!cont",
        .usage = "!cont",
        .help = "Continues execution (may not appear so if input is needed).",
        .f = [&](auto, auto &) -> bool { return true; }};
    return {command.name, command};
  }
};
