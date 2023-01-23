#include <iostream>
#include <source_location>
#include <string>

#include "token.hpp"

#include "assert.hpp"

void myAssert(const Token& token, std::string msg, std::source_location loc) {
  std::cerr << loc.file_name() << ':' << loc.line() << ' '
            << loc.function_name() << ":\n";
  if (token.type == Token::Type::EOFF)
    std::cerr << "line " << token.lineNum << ": location: at end: " << msg
              << '\n';
  else
    std::cerr << "line " << token.lineNum << ": location: " << token.lexeme
              << ": " << msg << '\n';
  std::exit(1);
}
