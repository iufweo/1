#include <fmt/core.h>

#include "interp.hpp"

template <typename... Ts>
[[noreturn]] void err(int eval, fmt::format_string<Ts...> fmt, Ts &&...args) {
  fmt::print(stderr, fmt, std::forward<Ts>(args)...);
  fmt::print("\n");
  std::exit(eval);
}

int main(int argc, char *argv[]) {
  Interp i;

  switch (argc) {
  case 1:
    i.runPrompt();
    break;
  case 2:
    i.runFile(argv[1]);
    break;
  default:
    err(1, "argc = {}", argc);
    break;
  }

  return 0;
}
