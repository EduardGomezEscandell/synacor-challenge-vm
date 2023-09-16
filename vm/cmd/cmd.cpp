#include <cstdlib>
#include <fstream>
#include <initializer_list>
#include <ios>
#include <iostream>
#include <array>
#include <istream>

#include "lib/cpu.hpp"


int main(int argc , char** argv) {
	if (argc != 2) {
		std::cerr << "Argc should be 2\n";
		exit(EXIT_FAILURE);
	}

  auto f = std::ifstream(argv[1], std::ios_base::binary);

  SynacorVM::CPU vm;
  vm.Load(f);
  vm.Run();

  f.close();

  return 0;
}
