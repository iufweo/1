#include <string>

#include "ltype.hpp"

#include "token.hpp"

using enum Token::Type;

Token::Token(Token::Type type,
             std::string lexeme,
             Literal literal,
             std::size_t lineNum)
    : type(type), lexeme(lexeme), literal(literal), lineNum(lineNum) {}

Token::operator std::string() const {
  static const char* t[] = {
      "LEFT_PAREN",    "RIGHT_PAREN", "LEFT_BRACE",  "RIGHT_BRACE",
      "COMMA",         "DOT",         "MINUS",       "PLUS",
      "SEMICOLON",     "SLASH",       "STAR",        "BANG",
      "BANG_EQUAL",    "EQUAL",       "EQUAL_EQUAL", "GREATER",
      "GREATER_EQUAL", "LESS",        "LESS_EQUAL",  "IDENTIFIER",
      "STRING",        "NUMBER",      "TRUE",        "FALSE",
      "AND",           "CLASS",       "ELSE",        "FUN",
      "FOR",           "IF",          "NIL",         "OR",
      "PRINT",         "RETURN",      "SUPER",       "THIS",
      "VAR",           "WHILE",       "EOFF",        "COLON",
      "QUESTIONMARK",  "CONTINUE",    "BREAK",       "PERCENT"};
  static_assert(sizeof(t) / sizeof(t[0]) ==
                (unsigned int)PERCENT - (unsigned int)LEFT_PAREN + 1);

  return std::string(
             t[static_cast<std::underlying_type_t<Token::Type>>(type)]) +
         " " + lexeme + " " + literalToString(literal);
}

bool Token::operator==(const Token& rhs) const {
  return lexeme == rhs.lexeme;
}

std::size_t Token::Hash::operator()(const Token& token) const noexcept {
  return std::hash<std::string>()(token.lexeme);
}
