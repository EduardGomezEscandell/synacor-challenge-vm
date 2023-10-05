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

  command_preprocessor p(std::cin, cov);
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
    std::cerr << std::format("Disabled {}\n", name) << std::flush;
    return;
  }

  other_prehooks[name] = std::forward<decltype(f) &&>(f);
  std::cerr << std::format("Enabled {}\n", name) << std::flush;
  return;
}



bool command_preprocessor::command(std::string cmd,
                                   SynacorVM::execution_state es) {
  std::stringstream ss{cmd};
  try {
    auto verb = next_word(ss);
    auto it = commands.find(verb);
    if (it == commands.end()) {
      std::cerr << std::format("Unkown instruction {}\n", verb) << std::flush;
      return false;
    }

    return it->second.f(es, ss);

  } catch (std::exception &e) {
    std::cerr << std::format("Failed to execute command '{}': {}\n", cmd,
                             e.what())
              << std::flush;
    return false;
  } catch (...) {
    std::cerr << std::format("Failed to execute command '{}'\n", cmd)
              << std::flush;
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
                << "Use !help for help and !cont to continue running the VM\n"
                << std::flush;
      first_instruction = false;
    } else if (sleep == 0 && !first_instruction) {
      --sleep;
      std::cerr << "\nWoke up after sleeping\n" << std::flush;
    } else if (dbg_point) {
      std::cerr << std::format("\nStopped at debug point {:04x}\n",
                               es.instruction_ptr.to_uint())
                << std::flush;
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
  ss << '\n';

  return ss.str();
}
