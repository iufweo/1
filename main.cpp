#include <iostream>

#include "interp.hpp"

int main(int argc, char* argv[]) {
  Interp i;

  switch (argc) {
    case 1:
      i.runPrompt();
      break;
    case 2:
      i.runFile(argv[1]);
      break;
    default:
      std::cerr << "argc = " << argc << '\n';
      std::exit(1);
      break;
  }

  return 0;
}
