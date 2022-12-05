#include <source_location>
#include <string>

#include <fmt/core.h>

#include "token.hpp"

#include "assert.hpp"

void myAssert(const Token &token, std::string msg, std::source_location loc) {
  fmt::print(stderr, "{}:{} {}:\n", loc.file_name(), loc.line(),
             loc.function_name());
  if (token.type == Token::Type::EOFF)
    fmt::print(stderr, "line {}: location: at end: {}\n", token.lineNum, msg);
  else
    fmt::print(stderr, "line {}: location: at '{}': {}\n", token.lineNum,
               token.lexeme, msg);
  std::exit(1);
}
