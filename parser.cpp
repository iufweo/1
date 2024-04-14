#include <list>
#include <memory>

#include "expr.hpp"
#include "interp.hpp"
#include "ltype.hpp"
#include "stmt.hpp"

#include "parser.hpp"

using enum Token::Type;

std::list<Token> Parser::tokenList;
decltype(Parser::tokenList.cbegin()) Parser::itCurrent;
decltype(Parser::tokenList.cbegin()) Parser::itPrevious;

template <typename T>
std::shared_ptr<const Expr> Parser::parseBinary(
    std::shared_ptr<const Expr> (*f)(),
    std::initializer_list<Token::Type> tts) {
  std::shared_ptr<const Expr> e, right;

  e = f();

  while (match(tts)) {
    Token oper = previous();
    right = f();
    e = std::make_shared<const T>(e, oper, right);
  }

  return e;
}

std::shared_ptr<const Expr> Parser::expression() {
  return comma();
}

std::shared_ptr<const Expr> Parser::assignment() {
  std::shared_ptr<const Expr> exprp, value;
  std::shared_ptr<const ExprVar> varExprp;
  std::shared_ptr<const ExprGet> getExprp;

  exprp = conditional();

  if (match({EQUAL})) {
    Token token = previous();
    value = assignment();

    if ((varExprp = std::dynamic_pointer_cast<const ExprVar>(exprp)) != nullptr)
      return std::make_shared<const ExprAssign>(varExprp->token, value);
    if ((getExprp = std::dynamic_pointer_cast<const ExprGet>(exprp)) != nullptr)
      // decompose like this due to immutable ExprGet
      return std::make_shared<const ExprSet>(getExprp, getExprp->token, value);

    error(token, "invalid assignment target");
  }

  return exprp;
}

std::shared_ptr<const Expr> Parser::comma() {
  return parseBinary<ExprComma>(assignment, {COMMA});
}

std::shared_ptr<const Expr> Parser::logicalOr() {
  return parseBinary<ExprLogical>(logicalAnd, {OR});
}

std::shared_ptr<const Expr> Parser::logicalAnd() {
  return parseBinary<ExprLogical>(equality, {AND});
}
// a == a ? b : (c ? d : e)
std::shared_ptr<const Expr> Parser::conditional() {
  std::shared_ptr<const Expr> exprp, thenp, elsep;

  exprp = logicalOr();
  while (match({QUESTIONMARK})) {
    thenp = expression();
    consume(COLON, "expected ':' after '?'");
    elsep = conditional();
    exprp = std::make_shared<const ExprTern>(exprp, thenp, elsep);
  }

  return exprp;
}

std::shared_ptr<const Expr> Parser::equality() {
  return parseBinary<ExprBinary>(comparison, {BANG_EQUAL, EQUAL_EQUAL});
}

std::shared_ptr<const Expr> Parser::comparison() {
  return parseBinary<ExprBinary>(term,
                                 {GREATER, GREATER_EQUAL, LESS, LESS_EQUAL});
}

std::shared_ptr<const Expr> Parser::term() {
  return parseBinary<ExprBinary>(factor, {MINUS, PLUS});
}

std::shared_ptr<const Expr> Parser::factor() {
  return parseBinary<ExprBinary>(unary, {SLASH, STAR, PERCENT});
}

std::shared_ptr<const Expr> Parser::unary() {
  std::shared_ptr<const Expr> right;

  while (match({BANG, MINUS})) {
    Token oper = previous();
    right = unary();
    return std::make_shared<const ExprUnary>(oper, right);
  }

  return call();
}

std::shared_ptr<const Expr> Parser::call() {
  std::shared_ptr<const Expr> exprp;

  exprp = primary();

  while (1) {
    if (match({LEFT_PAREN})) {
      exprp = finishCall(std::move(exprp));
    } else if (match({DOT})) {
      Token token = consume(IDENTIFIER, "expected identifier after '.'");
      exprp = std::make_shared<const ExprGet>(exprp, token);
    } else {
      break;
    }
  }

  return exprp;
}

std::shared_ptr<const Expr> Parser::finishCall(
    std::shared_ptr<const Expr> exprp) {
  std::list<std::shared_ptr<const Expr>> args;
  unsigned int argsCount;

  argsCount = 0;
  if (!check(RIGHT_PAREN)) {
    do {
      // not expression because expression == comma
      args.push_back(assignment());
      argsCount++;
      if (argsCount > N_ARGS_MAX)
        error(peek(),
              "the maximum arguments amount is " + std::to_string(N_ARGS_MAX));
    } while (match({COMMA}));
  }
  Token save = consume(RIGHT_PAREN, "expected ')' after arguments");
  return std::make_shared<const ExprCall>(exprp, save, std::move(args));
}

std::shared_ptr<const Expr> Parser::primary() {
  std::shared_ptr<const Expr> e;

  if (match({FALSE}))
    return std::make_shared<const ExprLiteral>(false);
  if (match({TRUE}))
    return std::make_shared<const ExprLiteral>(true);
  if (match({NIL}))
    return std::make_shared<const ExprLiteral>(Lnil());
  if (match({NUMBER, STRING}))
    return std::make_shared<const ExprLiteral>(
        literalToLtype(previous().literal));

  if (match({LEFT_PAREN})) {
    e = expression();
    consume(RIGHT_PAREN, "expected ')' after expression");
    return std::make_shared<const ExprGrouping>(e);
  }
  if (match({IDENTIFIER}))
    return std::make_shared<const ExprVar>(previous());
  if (match({THIS}))
    return std::make_shared<const ExprThis>(previous());
  if (match({SUPER})) {
    Token super = previous();
    consume(DOT, "expected '.' after 'super'");
    return std::make_shared<const ExprSuper>(
        super, consume(IDENTIFIER, "expected identifier after '.'"));
  }
  if (match({FUN}))
    return expressionFun();
  if (match({EQUAL_EQUAL, BANG_EQUAL}))
    throw handleErrorProduction(equality);
  if (match({GREATER, GREATER_EQUAL, LESS, LESS_EQUAL}))
    throw handleErrorProduction(comparison);
  if (match({MINUS, PLUS}))
    throw handleErrorProduction(term);
  if (match({SLASH, STAR, PERCENT}))
    throw handleErrorProduction(factor);
  // COLON, QUESTIONMARK, COMMA

  throw error(peek(), "expected expression");
}

std::shared_ptr<const ExprFun> Parser::expressionFun() {
  std::tuple<std::list<Token>, std::unique_ptr<const StmtList>> t;
  consume(LEFT_PAREN, "expected '(' for function expression");
  t = parseFun();
  return std::make_shared<const ExprFun>(std::get<0>(t),
                                         std::move(std::get<1>(t)));
}

bool Parser::check(Token::Type type) {
  if (isAtEnd())
    return 0;
  return peek().type == type;
}

const Token& Parser::advance() {
  if (!isAtEnd())
    itPrevious = itCurrent++;
  else
    itPrevious = itCurrent;
  return previous();
}

const Token& Parser::peek() {
  return *itCurrent;
}

const Token& Parser::previous() {
  return *itPrevious;
}

bool Parser::isAtEnd() {
  return peek().type == EOFF;
}

bool Parser::match(std::initializer_list<Token::Type> tts) {
  for (Token::Type tt : tts) {
    if (check(tt)) {
      advance();
      return 1;
    }
  }

  return 0;
}

const Token& Parser::consume(Token::Type tt, std::string msg) {
  if (!check(tt))
    throw error(peek(), msg);
  return advance();
}

Parser::ParseError Parser::error(const Token& token, std::string msg) {
  Interp::error(token, msg);
  return ParseError();
}

void Parser::synchronize() {
  advance();

  while (!isAtEnd()) {
    if (previous().type == SEMICOLON)
      return;

    switch (peek().type) {
      case CLASS:
      case FUN:
      case VAR:
      case FOR:
      case IF:
      case WHILE:
      case PRINT:
      case RETURN:  // FALLTHROUGH
        return;
        break;
      default:
        break;
    }
    advance();
  }
}

std::list<std::shared_ptr<const Stmt>> Parser::parse(
    const std::list<Token>& tokenList) {
  std::list<std::shared_ptr<const Stmt>> stmtPList;
  std::unique_ptr<const Stmt> stmtp;

  Parser::tokenList = tokenList;
  itCurrent = Parser::tokenList.cbegin();
  itPrevious = itCurrent;

  while (!isAtEnd()) {
    if ((stmtp = definition()) != nullptr)
      stmtPList.push_back(std::move(stmtp));
  }

  return stmtPList;
}

Parser::ParseError Parser::handleErrorProduction(
    std::shared_ptr<const Expr> (*f)()) {
  // skip the next expression with the appropriate precedence
  f();
  Interp::error(peek(), "expected expression before the operator");
  return ParseError();
}

std::unique_ptr<const Stmt> Parser::statement() {
  if (match({PRINT}))
    return statementPrint();
  if (match({LEFT_BRACE}))
    return std::make_unique<const StmtList>(statementList());
  if (match({IF}))
    return statementIf();
  if (match({WHILE}))
    return statementWhile();
  if (match({FOR}))
    return statementFor();
  if (match({BREAK, CONTINUE}))
    return statementLoopFlow();
  if (match({RETURN}))
    return statementReturn();
  return statementExpression();
}

std::unique_ptr<const Stmt> Parser::statementPrint() {
  std::shared_ptr<const Expr> e;

  e = expression();
  consume(SEMICOLON, "expected ';' after expression");
  return std::make_unique<const StmtPrint>(e);
}

std::unique_ptr<const Stmt> Parser::statementExpression() {
  std::shared_ptr<const Expr> e;

  e = expression();
  consume(SEMICOLON, "expected ';' after expression");
  return std::make_unique<const StmtExpr>(e);
}

void Parser::back() {
  if (itPrevious != tokenList.begin())
    itCurrent = itPrevious--;
  else
    itCurrent = itPrevious;
}

bool Parser::matchTwo(Token::Type first, Token::Type second) {
  if (check(first)) {
    advance();
    if (check(second))
      return 1;
    else
      back();
  }
  return 0;
}

std::unique_ptr<const Stmt> Parser::definition() {
  std::unique_ptr<const Stmt> stmtp;

  try {
    if (match({VAR}))
      stmtp = definitionVar();
    else if (matchTwo(FUN, IDENTIFIER))
      stmtp = definitionFun();
    else if (match({CLASS}))
      stmtp = definitionClass();
    else
      stmtp = statement();
  } catch (ParseError& e) {
    synchronize();

    stmtp = nullptr;
  }

  return stmtp;
}

std::unique_ptr<const Stmt> Parser::definitionVar() {
  std::shared_ptr<const Expr> initExpr;

  initExpr = nullptr;

  Token t = consume(IDENTIFIER, "expected identifier");
  if (match({EQUAL}))
    initExpr = expression();

  consume(SEMICOLON, "expected terminating ';'");
  return std::make_unique<const StmtVar>(t, initExpr);
}

std::list<std::shared_ptr<const Stmt>> Parser::statementList() {
  std::list<std::shared_ptr<const Stmt>> list;

  while (peek().type != RIGHT_BRACE && !isAtEnd())
    list.push_back(std::shared_ptr<const Stmt>(definition()));
  consume(RIGHT_BRACE, "expected '}' for list");
  return list;
}

std::unique_ptr<const Stmt> Parser::statementIf() {
  std::shared_ptr<const Expr> condp;
  std::unique_ptr<const Stmt> thenp, elsep;

  elsep = nullptr;

  consume(LEFT_PAREN, "expected '(' after 'if'");
  condp = expression();
  consume(RIGHT_PAREN, "expected ')' after expression");
  thenp = statement();
  if (match({ELSE}))
    elsep = statement();

  return std::make_unique<const StmtIf>(condp, thenp.release(),
                                        elsep.release());
}

std::unique_ptr<const Stmt> Parser::statementWhile() {
  std::shared_ptr<const Expr> condp;
  std::unique_ptr<const Stmt> stmtp;

  consume(LEFT_PAREN, "expected '(' after while");
  condp = expression();
  consume(RIGHT_PAREN, "expected ')'");
  stmtp = statement();

  return std::make_unique<const StmtLoop>(condp, nullptr, stmtp.release());
}

std::unique_ptr<const Stmt> Parser::statementFor() {
  std::list<std::shared_ptr<const Stmt>> body;
  std::list<std::shared_ptr<const Stmt>> wrap;
  std::shared_ptr<const Expr> condp, exprp;
  std::unique_ptr<const Stmt> declp, stmtp;
  std::unique_ptr<const StmtLoop> loopp;

  condp = nullptr;
  exprp = nullptr;
  declp = nullptr;

  consume(LEFT_PAREN, "expected '(' after 'for'");

  if (match({SEMICOLON})) {
    ;
  } else if (match({VAR})) {
    declp = definitionVar();
  } else {
    declp = statementExpression();
  }
  // for ( ; ; )
  if (match({SEMICOLON})) {
    condp = std::make_shared<const ExprLiteral>(true);
  } else {
    condp = expression();
    consume(SEMICOLON, "expected ';' after expression");
  }

  if (match({RIGHT_PAREN})) {
    ;
  } else {
    exprp = expression();
    consume(RIGHT_PAREN, "expected ')'");
  }
  body.push_back(statement());
  // nullptr exprp is handled
  loopp = std::make_unique<const StmtLoop>(condp, exprp,
                                           new StmtList(std::move(body)));
  if (declp != nullptr) {
    wrap.push_back(std::move(declp));
    wrap.push_back(std::move(loopp));
    return std::make_unique<const StmtList>(std::move(wrap));
  }

  return loopp;
}

std::unique_ptr<const Stmt> Parser::statementLoopFlow() {
  std::unique_ptr<const StmtLoopFlow> stmtp;

  stmtp = std::make_unique<const StmtLoopFlow>(previous());
  consume(SEMICOLON, "expected ';'");
  return stmtp;
}

std::tuple<std::list<Token>, std::unique_ptr<const StmtList>>
Parser::parseFun() {
  std::list<Token> params;
  unsigned int paramsCount;

  paramsCount = 0;
  if (!check(RIGHT_PAREN)) {
    do {
      params.push_back(consume(IDENTIFIER, "expected parameter name"));
      paramsCount++;
      if (paramsCount > N_ARGS_MAX)
        error(peek(),
              "the maximum parameter amount is " + std::to_string(N_ARGS_MAX));
    } while (match({COMMA}));
  }

  consume(RIGHT_PAREN, "expected ')' after parameters");
  consume(LEFT_BRACE, "expected '{' at the start of function body");

  return {params, std::make_unique<const StmtList>(statementList())};
}

std::unique_ptr<const StmtFun> Parser::definitionFun() {
  std::tuple<std::list<Token>, std::unique_ptr<const StmtList>> t;

  Token token = consume(IDENTIFIER,
                        "expected identifier after 'fun' "
                        "for function definition");
  consume(LEFT_PAREN, "expected '(' after '" + token.lexeme +
                          "' for "
                          "function definition");
  t = parseFun();
  return std::make_unique<const StmtFun>(token, std::get<0>(t),
                                         std::move(std::get<1>(t)));
}

std::unique_ptr<const Stmt> Parser::statementReturn() {
  std::shared_ptr<const Expr> exprp;

  exprp = nullptr;

  Token token = previous();
  if (peek().type != SEMICOLON)
    exprp = expression();
  consume(SEMICOLON, "expected ';'");
  return std::make_unique<const StmtReturn>(token, std::move(exprp));
}

std::unique_ptr<const Stmt> Parser::definitionClass() {
  std::list<std::shared_ptr<const StmtFun>> methods;
  std::list<std::shared_ptr<const StmtFun>> staticMethods;
  std::unique_ptr<const StmtFun> funp;
  std::unique_ptr<const StmtFun> ctor;
  std::shared_ptr<const ExprVar> superExpr;

  ctor = nullptr;
  superExpr = nullptr;

  Token token = consume(IDENTIFIER, "expected identifier after 'class'");
  if (match({LESS})) {
    consume(IDENTIFIER, "expected class name after '<'");
    superExpr = std::make_shared<const ExprVar>(previous());
  }
  consume(LEFT_BRACE, "expected '}' before class body");

  while (match({FUN, CLASS})) {
    if (previous().type == FUN) {
      funp = definitionFun();
      // if optional ctor
      if (funp->token.lexeme == token.lexeme)
        ctor = std::move(funp);
      else
        methods.push_back(std::move(funp));
    } else {
      consume(FUN,
              "expected 'fun' after 'class' for static "
              "method");
      // may have the same name as the class, leave to
      // resolver
      staticMethods.push_back(definitionFun());
    }
  }

  consume(RIGHT_BRACE, "expected '}' after class body");

  return std::make_unique<const StmtClass>(token, std::move(superExpr),
                                           std::move(ctor), std::move(methods),
                                           std::move(staticMethods));
}

Parser::ParseError::ParseError() : runtime_error("") {}
