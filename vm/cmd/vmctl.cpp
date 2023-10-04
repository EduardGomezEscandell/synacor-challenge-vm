#include <csignal>
#include <cstdlib>
#include <format>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>

#include "arch/arch.hpp"
#include "lib/cpu.hpp"
#include "lib/memory.hpp"
#include "lib/word.hpp"

#include "helpers.hpp"
#include "vmctl.hpp"

std::unique_ptr<coverage> cov(nullptr);

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Argc should be 2\n";
    exit(EXIT_FAILURE);
  }

  ::signal(SIGINT, [](int) {
    std::cout << std::endl;
    std::cerr << std::endl;

    if (cov.get() != nullptr) {
      std::cerr << cov->summary() << std::flush;
    }
    exit(EXIT_FAILURE);
  });

  SynacorVM::Memory ram;
  SynacorVM::CPU vm{.memory = ram};

  command_preprocessor p(std::cin);
  p.install(vm);

  ram.load(read_binary(argv[1]));

  vm.Run();

  if (cov.get() != nullptr) {
    std::cerr << cov->summary() << std::flush;
  }
  return 0;
}

void command_preprocessor::toggle_hook(
    std::string const &name,
    std::function<void(SynacorVM::execution_state)> &&f) {
  if (auto it = other_prehooks.find(name); it != other_prehooks.end()) {
    other_prehooks.erase(it);
    std::cerr << std::format("Disabled {}", name) << std::endl;
    return;
  }

  other_prehooks[name] = std::forward<decltype(f) &&>(f);
  std::cerr << std::format("Enabled {}", name) << std::endl;
  return;
}

auto next(std::stringstream &ss) -> std::string {
  std::string v;
  ss >> v;
  return v;
};

std::map<std::string, bool (*)(command_preprocessor &,
                               SynacorVM::execution_state, std::stringstream &)>
    commands{{"!setr",
              [](auto &, auto es, auto &argstream) -> bool {
                const auto reg = std::stoul(next(argstream), nullptr, 0);
                const auto value = std::stoul(next(argstream), nullptr, 0);
                es.registers.at(reg) = SynacorVM::Word(value);
                std::cerr << std::format("Set register {} to 0x{:04x}", reg,
                                         value)
                          << std::endl;
                return false;
              }},
             {"!sleep",
              [](auto &p, auto, auto &argstream) -> bool {
                const auto s = std::stoll(next(argstream), nullptr, 0);
                p.set_sleep(s);
                std::cerr << std::format(
                                 "You'll be prompted again in {} instructions "
                                 "(Unless input is needed before)",
                                 s)
                          << std::endl;
                return true;
              }},
             {"!step",
              [](auto &p, auto, auto &) -> bool {
                p.set_sleep(1);
                return true;
              }},
             {"!dbg",
              [](auto &p, auto, auto &argstream) -> bool {
                const auto s = std::stoul(next(argstream), nullptr, 0);
                if (p.toggle_dbg_point(s)) {
                  std::cerr << std::format("Added debug point at {:04x}", s)
                            << std::endl;
                } else {
                  std::cerr << std::format("Removed debug point at {:04x}", s)
                            << std::endl;
                }
                return false;
              }},
             {"!getr",
              [](auto &, auto es, auto &) -> bool {
                for (auto &r : es.registers) {
                  std::cerr << std::format("  {:04x}", r.to_uint());
                }
                std::cerr << std::endl;
                return false;
              }},
             {"!instr",
              [](auto &p, auto, auto &) -> bool {
                p.toggle_hook("instruction printing", [](auto es) {
                  std::cerr << peek_instruction(es) << std::endl;
                });
                return false;
              }},
             {"!exit",
              [](auto &p, auto es, auto &) -> bool {
                es.heap[es.instruction_ptr.to_uint()] =
                    SynacorVM::Word(static_cast<unsigned>(Verb::HALT));
                std::cerr << "Exiting" << std::endl;
                p.enqueue(std::char_traits<char>::eof());
                return false;
              }},
             {"!covr",
              [](auto &p, auto, auto) -> bool {
                if (cov.get() == nullptr) {
                  cov = std::make_unique<coverage>();
                }
                p.toggle_hook("compute coverage", cov->pre_exec_hook());
                return false;
              }},
             {"!help", [](auto &, auto, auto &) -> bool {
                std::cerr << "Use any of these commads:\n"
                          << "You can escape the leading ! by writting !!"
                          << std::endl;
                for (auto const &cmd : commands) {
                  std::cerr << " > " << cmd.first << '\n';
                }
                std::cerr << std::flush;
                return false;
              }}};

bool command_preprocessor::command(std::string cmd,
                                   SynacorVM::execution_state es) {
  std::stringstream ss{cmd};
  try {
    auto verb = next(ss);
    auto it = commands.find(verb);
    if (it == commands.end()) {
      std::cerr << std::format("Unkown instruction {}", verb) << std::endl;
      return false;
    }

    return it->second(*this, es, ss);

  } catch (std::exception &e) {
    std::cerr << std::format("Failed to execute command '{}': {}", cmd,
                             e.what())
              << std::endl;
    return false;
  } catch (...) {
    std::cerr << std::format("Failed to execute command '{}'", cmd)
              << std::endl;
    return false;
  }
}

void command_preprocessor::install(SynacorVM::CPU &target) {
  assert(cpu == nullptr);

  cpu = &target;
  cpu->stdIn = &out;
  cpu->pre_exec_hook = [this](auto state) { this->pre_exec_hook(state); };
}

void command_preprocessor::pre_exec_hook(SynacorVM::execution_state es) {
  for (auto const &hook : other_prehooks) {
    hook.second(es);
  }

  auto opcode = es.heap[es.instruction_ptr.to_uint()].to_uint();
  const bool dbg_point = debug_points.contains(es.instruction_ptr.to_uint());
  if (opcode != Verb::IN && sleep != 0 && !dbg_point) {
    --sleep;
    return;
  }

  if (opcode == Verb::IN) {
    queued_chars = queued_chars == 0 ? 0 : queued_chars - 1;
  }

  if (queued_chars > 0 && sleep != 0 && !dbg_point) {
    --sleep;
    return;
  }

  while (queued_chars == 0 || opcode != Verb::IN) {
    std::string buff;

    if (in.eof()) {
      enqueue(std::char_traits<char>::eof());
      return;
    }

    if (first_instruction) {
      std::cerr << "This is your chance to pre-populate the input.\n"
                << "Use !help for help and !cont to continue running the VM"
                << std::endl;
      first_instruction = false;
    } else if (sleep == 0 && !first_instruction) {
      --sleep;
      std::cerr << "\nWoke up after sleeping" << std::endl;
    } else if (dbg_point) {
      std::cerr << std::format("\nStopped at debug point {:04x}",
                               es.instruction_ptr.to_uint())
                << std::endl;
    }

    std::getline(in, buff);

    if (buff == "!cont" && opcode != Verb::IN) {
      --sleep;
      return;
    }

    bool is_command = buff.starts_with("!");
    if (is_command && buff.starts_with("!!")) {
      is_command = false;
      buff = std::string{buff.begin() + 1, buff.end()};
    }

    if (!is_command) {
      // Not a command
      out << buff << '\n';
      queued_chars = buff.size() + 1;
      return;
    }

    if (const bool cont = command(buff, es); cont && opcode != Verb::IN) {
      return;
    } else {
      continue;
    }
  }
}

std::string parse_value(SynacorVM::Word w) {
  const auto v = w.to_uint();
  if (v >= 0x8000 && v < 0x8008) {
    // Register
    return std::format("r{}", static_cast<int>(v - 0x8000));
  }

  if (v >= ' ' && v <= '~') {
    // Literal character
    return std::format("'{}'", static_cast<char>(v));
  }

  return std::format("{:x}", v);
}

std::string peek_instruction(SynacorVM::execution_state es) {
  const SynacorVM::Word verb = es.heap[es.instruction_ptr.to_uint()];

  const auto v = static_cast<Verb>(verb.to_uint());

  std::stringstream ss;

  ss << std::format("0x{:04x} | {}", es.instruction_ptr.to_uint(),
                    arch::to_string(v));
  const auto argc = arch::argument_count(v);
  if (argc < 0) {
    ss << std::format("UNKNOWN {:x}\n", verb.to_uint());
    return ss.str();
  }

  for (auto i = 0u; i < unsigned(argc); ++i) {
    ss << ' ' << parse_value(es.heap[es.instruction_ptr.to_uint() + 1 + i]);
  }

  ss << " | ";
  for (auto &r : es.registers) {
    ss << std::format("  {:04x}", r.to_uint());
  }

  return ss.str();
}
