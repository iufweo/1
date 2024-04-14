#include <iostream>

#include "interp.hpp"

int main(int argc, char* argv[]) {
  Interp i;
  int ret;

  switch (argc) {
    case 1:
      i.runPrompt();
      ret = 0;
      break;
    case 2:
      ret = i.runFile(argv[1]);
      break;
    default:
      std::cerr << "argc = " << argc << '\n';
      std::exit(1);
      break;
  }

  return ret;
}
