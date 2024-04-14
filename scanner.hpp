#pragma once
#include <list>
#include <string>
#include <unordered_map>

#include "token.hpp"
#include "uncopyable.hpp"

class Scanner : public Uncopyable {
 private:
  std::string input;
  std::size_t start, current, lineNum;
  std::list<Token> tokenList;

  static const std::unordered_map<std::string, const Token::Type> keywords;
  void scanToken();
  bool compareAndNext(char expected);

  void addToken(Token::Type tt);
  void addToken(Token::Type tt, Literal literal);

 public:
  Scanner(std::string inputStr);

  std::list<Token> scanTokens();
};
