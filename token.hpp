#pragma once
#include <string>

#include "ltype.hpp"

struct Token {
  enum class Type : unsigned int {
    LEFT_PAREN = 0,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    COMMA,
    DOT,
    MINUS,
    PLUS,
    SEMICOLON,
    SLASH,
    STAR,

    BANG,
    BANG_EQUAL,
    EQUAL,
    EQUAL_EQUAL,
    GREATER,
    GREATER_EQUAL,
    LESS,
    LESS_EQUAL,

    IDENTIFIER,
    STRING,
    NUMBER,
    TRUE,
    FALSE,
    AND,
    CLASS,
    ELSE,
    FUN,
    FOR,
    IF,
    NIL,
    OR,
    PRINT,
    RETURN,
    SUPER,
    THIS,
    VAR,
    WHILE,
    // EOF is reserved
    EOFF,
    COLON,
    QUESTIONMARK,
    CONTINUE,
    BREAK,
    PERCENT
  } type;
  std::string lexeme;
  Literal literal;
  std::size_t lineNum;

  Token(Token::Type type,
        std::string lexeme,
        Literal literal,
        std::size_t lineNum);
  Token() = delete;
  operator std::string() const;

  struct Hash {
    std::size_t operator()(const Token& token) const noexcept;
  };
  bool operator==(const Token& rhs) const;
};
