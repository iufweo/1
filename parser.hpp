#pragma once
#include <list>
#include <memory>
#include <stdexcept>
#include <string>

#include "expr_fwd.hpp"
#include "stmt_fwd.hpp"
#include "uncopyable.hpp"

#include "ltype.hpp"
#include "token.hpp"

const unsigned int N_ARGS_MAX = 255;

class Parser : public Uncopyable {
 private:
  static std::list<Token> tokenList;
  static decltype(tokenList.cbegin()) itCurrent;
  static decltype(tokenList.cbegin()) itPrevious;

  static bool match(std::initializer_list<Token::Type> tts);
  static bool check(Token::Type type);
  static const Token& peek();
  static const Token& advance();
  static bool isAtEnd();
  static const Token& previous();
  static const Token& consume(Token::Type tt, std::string msg);
  static bool matchTwo(Token::Type first, Token::Type second);
  static void back();

  static std::shared_ptr<const Expr> expression();
  static std::shared_ptr<const Expr> assignment();
  static std::shared_ptr<const Expr> logicalOr();
  static std::shared_ptr<const Expr> logicalAnd();
  static std::shared_ptr<const Expr> comma();
  static std::shared_ptr<const Expr> conditional();
  static std::shared_ptr<const Expr> equality();
  static std::shared_ptr<const Expr> comparison();
  static std::shared_ptr<const Expr> term();
  static std::shared_ptr<const Expr> factor();
  static std::shared_ptr<const Expr> unary();
  static std::shared_ptr<const Expr> call();
  static std::shared_ptr<const Expr> finishCall(
      std::shared_ptr<const Expr> exprp);
  static std::shared_ptr<const Expr> primary();
  static std::shared_ptr<const ExprFun> expressionFun();

  template <typename T>
  static std::shared_ptr<const Expr> parseBinary(
      std::shared_ptr<const Expr> (*f)(),
      std::initializer_list<Token::Type> tts);
  static std::tuple<std::list<Token>, std::unique_ptr<const StmtList>>
  parseFun();

  static std::unique_ptr<const Stmt> statement();
  static std::unique_ptr<const Stmt> statementPrint();
  static std::unique_ptr<const Stmt> statementExpression();
  static std::unique_ptr<const Stmt> definition();
  static std::unique_ptr<const Stmt> definitionVar();
  static std::list<std::shared_ptr<const Stmt>> statementList();
  static std::unique_ptr<const Stmt> statementIf();
  static std::unique_ptr<const Stmt> statementWhile();
  static std::unique_ptr<const Stmt> statementFor();
  static std::unique_ptr<const Stmt> statementLoopFlow();
  static std::unique_ptr<const StmtFun> definitionFun();
  static std::unique_ptr<const Stmt> statementReturn();
  static std::unique_ptr<const Stmt> definitionClass();

  static void synchronize();

  class ParseError : public std::runtime_error {
   public:
    ParseError();
  };

  static ParseError error(const Token& token, std::string msg);
  static ParseError handleErrorProduction(std::shared_ptr<const Expr> (*f)());

 public:
  Parser() = delete;

  static std::list<std::shared_ptr<const Stmt>> parse(
      const std::list<Token>& tokenList);
};
