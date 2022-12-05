#include <iostream>
#include <list>
#include <string>
// floating-point arithmetic
#include <cmath>
#include <memory>
// measure run time
//#include <chrono>
// for readAll -- reading from a file
#include <fstream>
#include <optional>
#include <variant>

#include <fmt/core.h>

#include "assert.hpp"
#include "func.hpp"
#include "io.hpp"
#include "parser.hpp"
#include "resolver.hpp"
#include "scanner.hpp"
#include "token.hpp"

#include "interp.hpp"

using enum Token::Type;

bool Interp::floatcmp(double a, double b) {
  const double absEps = 1e-15;
  const double relEps = 1e-14;
  double diff;

  if ((diff = std::abs(a - b)) < absEps)
    return 1;
  return diff <= std::max(std::abs(a), std::abs(b)) * relEps;
}

template <typename T> constexpr bool always_false_v = false;

bool Interp::equality(const Ltype &left, const Ltype &right) {
  bool ret;

  std::visit(
      [this, &ret](auto x, auto y) -> void {
        using T = decltype(x);
        using U = decltype(y);

        if constexpr (!std::is_same_v<T, U>) {
          ret = 0;
        } else {
          if constexpr (std::is_same_v<T, Lstring> || std::is_same_v<T, bool>)
            ret = x == y;
          else if constexpr (std::is_same_v<T, double>)
            ret = floatcmp(x, y);
          else if constexpr (std::is_same_v<T, Lnil>)
            ret = 1;
          else if constexpr (std::is_same_v<T, FunPtr> ||
                             std::is_same_v<T, LfunPtr> ||
                             std::is_same_v<T, InstPtr> ||
                             std::is_same_v<T, ClassPtr>)
            ret = !std::less()(x, y) && !std::less()(y, x);
          else
            static_assert(always_false_v<T>);
        }
      },
      left, right);

  return ret;
}

Ltype Interp::visit(std::shared_ptr<const ExprBinary> expr) {
  Ltype ret, left, right;
  ReclaimerCtx ctx(*this);

  left = ctx.add(eval(expr->left));
  right = ctx.add(eval(expr->right));

  switch (expr->oper.type) {
  case MINUS:
    checkNumberOperands(expr->oper, left, right);
    ret = std::get<double>(left) - std::get<double>(right);
    break;
  case STAR:
    checkNumberOperands(expr->oper, left, right);
    ret = std::get<double>(left) * std::get<double>(right);
    break;
  case SLASH:
    checkNumberOperands(expr->oper, left, right);
    if (floatcmp(std::get<double>(right), 0.0))
      throw RuntimeError(expr->oper, "division by zero");
    ret = std::get<double>(left) / std::get<double>(right);
    break;
  case PERCENT:
    checkNumberOperands(expr->oper, left, right);
    if (floatcmp(std::get<double>(right), 0.0))
      throw RuntimeError(expr->oper, "division by zero");

    ret = std::fmod(std::get<double>(left), std::get<double>(right));
    break;
  case PLUS:
    if (std::holds_alternative<double>(left) &&
        std::holds_alternative<double>(right))
      ret = std::get<double>(left) + std::get<double>(right);
    else if (std::holds_alternative<Lstring>(left) &&
             std::holds_alternative<Lstring>(right))
      ret = std::get<Lstring>(left) + std::get<Lstring>(right);
    else
      throw RuntimeError(expr->oper,
                         "operands must be numbers or strings, got: " +
                             typeToString(left) + ", " + typeToString(right));
    break;
  case EQUAL_EQUAL:
    ret = equality(left, right);
    break;
  case BANG_EQUAL:
    ret = !equality(left, right);
    break;
  case LESS:
    checkNumberOperands(expr->oper, left, right);
    ret = std::get<double>(left) < std::get<double>(right);
    break;
  case LESS_EQUAL:
    checkNumberOperands(expr->oper, left, right);
    ret = std::get<double>(left) <= std::get<double>(right);
    break;
  case GREATER:
    checkNumberOperands(expr->oper, left, right);
    ret = std::get<double>(left) > std::get<double>(right);
    break;
  case GREATER_EQUAL:
    checkNumberOperands(expr->oper, left, right);
    ret = std::get<double>(left) >= std::get<double>(right);
    break;
  default:
    myAssert(expr->oper, "unhandled binary operator");
    break;
  }

  return ret;
}

Ltype Interp::visit(std::shared_ptr<const ExprComma> expr) {
  static_cast<void>(eval(expr->left));
  return eval(expr->right);
}

Ltype Interp::visit(std::shared_ptr<const ExprLogical> expr) {
  Ltype ret, left, right;
  ReclaimerCtx ctx(*this);

  left = ctx.add(eval(expr->left));

  switch (expr->oper.type) {
  case OR:
    if (isTruthful(left))
      ret = left;
    else
      ret = eval(expr->right);
    break;
  case AND:
    if (!isTruthful(left))
      ret = left;
    else
      ret = eval(expr->right);
    break;
  default:
    myAssert(expr->oper, "unhandled logical operator");
    break;
  }

  return ret;
}

Ltype Interp::visit(std::shared_ptr<const ExprGrouping> expr) {
  return eval(expr->exprp);
}

Ltype Interp::visit(std::shared_ptr<const ExprLiteral> expr) {
  return expr->value;
}

Ltype Interp::visit(std::shared_ptr<const ExprUnary> expr) {
  Ltype right, ret;

  right = eval(expr->exprp);

  switch (expr->oper.type) {
  case MINUS:
    checkNumberOperands(expr->oper, right);
    ret = -std::get<double>(right);
    break;
  case BANG:
    ret = !isTruthful(right);
    break;
  default:
    myAssert(expr->oper, "unhandled unary operator");
    break;
  }

  return ret;
}

Ltype Interp::visit(std::shared_ptr<const ExprTern> expr) {
  ReclaimerCtx ctx(*this);
  Ltype res;

  res = ctx.add(eval(expr->cond));
  if (isTruthful(res))
    return eval(expr->thenp);
  return eval(expr->elsep);
}

Ltype Interp::visit(std::shared_ptr<const ExprVar> expr) {
  return lookupVariable(expr->token, expr);
}

Ltype Interp::visit(std::shared_ptr<const ExprAssign> expr) {
  Ltype value;

  auto distance = locals.find(expr.get());
  value = eval(expr->exprp);
  if (distance != locals.end())
    envp->assignAt(expr->token, value, distance->second);
  else
    global.assign(expr->token, value);
  return value;
}

Ltype Interp::visit(std::shared_ptr<const ExprCall> expr) {
  Ltype callee;
  FunPtr ptr;
  std::list<Ltype> evaluatedArgs;
  ReclaimerCtx ctx(*this);

  ctx.add(callee = eval(expr->exprp));

  std::visit(
      [&ptr, &expr, &callee](auto x) -> void {
        using T = decltype(x);

        if constexpr (std::is_convertible_v<T, FunPtr>) {
          ptr = x;
        } else {
          throw RuntimeError(expr->savedParen,
                             "call to " + typeToString(callee) +
                                 ": can only call functions and constructors");
        }
      },
      callee);
  if (ptr->arity != expr->args.size())
    throw RuntimeError(expr->savedParen, "expected " +
                                             std::to_string(ptr->arity) +
                                             " arguments, got " +
                                             std::to_string(expr->args.size()));

  for (const std::shared_ptr<const Expr> &exprp : expr->args)
    evaluatedArgs.push_back(ctx.add(eval(exprp)));
  return ptr->call(*this, evaluatedArgs);
}

Ltype Interp::visit(std::shared_ptr<const ExprGet> expr) {
  Ltype obj;
  std::optional<Ltype> ret;
  ReclaimerCtx ctx(*this);

  obj = ctx.add(eval(expr->exprp));

  if (std::holds_alternative<InstPtr>(obj))
    ret = std::get<InstPtr>(obj)->get(*this, expr->token.lexeme);
  else if (std::holds_alternative<ClassPtr>(obj))
    // treat as an access to a static method
    ret = std::get<ClassPtr>(obj)->getStaticMethod(expr->token.lexeme);
  else
    throw RuntimeError(expr->token, "property access on a non-class object");
  if (!ret.has_value())
    throw RuntimeError(expr->token, "undefined property");
  return ret.value();
}

Ltype Interp::visit(std::shared_ptr<const ExprSet> expr) {
  Ltype obj;
  Ltype rvalue;
  ReclaimerCtx ctx(*this);

  obj = ctx.add(eval(expr->get->exprp));

  if (!std::holds_alternative<InstPtr>(obj))
    throw RuntimeError(expr->token, "only class instances have fields");
  rvalue = ctx.add(eval(expr->exprp));
  std::get<InstPtr>(obj)->set(expr->token.lexeme, rvalue);
  return rvalue;
}

Ltype Interp::visit(std::shared_ptr<const ExprThis> expr) {
  return lookupVariable(expr->token, expr);
}

Ltype Interp::visit(std::shared_ptr<const ExprSuper> expr) {
  ClassPtr superPtr;
  std::optional<Lfunc *> method;
  std::size_t distance;

  distance = locals[expr.get()];
  superPtr = std::get<ClassPtr>(envp->getAt(expr->token, distance));
  if ((method = superPtr->getMethod(expr->method.lexeme)).has_value())
    return method.value()->bind(
        *this, std::get<InstPtr>(envp->getAt(
                   Token(Token::Type::THIS, "this", "", 0), distance - 1)));
  method = superPtr->getStaticMethod(expr->method.lexeme);
  if (method.has_value())
    return method.value();
  throw RuntimeError(expr->method, "undefined property");
}

Ltype Interp::visit(std::shared_ptr<const ExprFun> expr) {
  ReclaimerCtx ctx(*this);
  return alloc<Lfunc>(ctx, expr, envp, false);
}

Ltype Interp::eval(std::shared_ptr<const Expr> expr) {
  return expr->accept(*this);
}

bool Interp::isTruthful(const Ltype &obj) const {
  if ((std::holds_alternative<bool>(obj) && std::get<bool>(obj) == 0) ||
      std::holds_alternative<Lnil>(obj))
    return 0;
  return 1;
}

void Interp::checkNumberOperands(Token oper, const Ltype &obj) const {
  if (!std::holds_alternative<double>(obj))
    throw RuntimeError(oper,
                       "operand must be a number, got: " + typeToString(obj));
}

void Interp::checkNumberOperands(Token oper, const Ltype &left,
                                 const Ltype &right) const {
  if (!std::holds_alternative<double>(left) ||
      !std::holds_alternative<double>(right))
    throw RuntimeError(oper,
                       "operands must be numbers, got: " + typeToString(left) +
                           ", " + typeToString(right));
}

void Interp::interpret(const std::list<std::shared_ptr<const Stmt>> &list) {
  try {
    for (const std::shared_ptr<const Stmt> &s : list)
      execute(*s.get());
  } catch (RuntimeError &e) {
    handleRuntimeError(e);
  }
}

void Interp::execute(const Stmt &stmt) { stmt.accept(*this); }

void Interp::execute(const StmtList &stmt, Env *setEnv) {
  Env *save;

  save = envp;
  envp = setEnv;
  for (const std::shared_ptr<const Stmt> &p : stmt.stmts) {
    try {
      execute(*p.get());
    } catch (std::exception &ex) {
      envp = save;
      throw;
    }
  }
  envp = save;
}

void Interp::visit(const StmtList &stmt) {
  ReclaimerCtx ctx(*this);
  execute(stmt, alloc<Env>(ctx, envp));
}

Interp::Env::Env() : enclosing(nullptr), m{} {}

Interp::Env::Env(Env *enclosing) : enclosing(enclosing), m{} {}

void Interp::Env::def(const Token &token, std::optional<Ltype> obj) {
  // allow redeclaring in global -- for REPL
  if (enclosing != nullptr) {
    // case 1:		case 2:
    // var x;		var x;	== nullopt
    // var x = 5;	var x;	== nullopt again
    auto ret = m.find(token.lexeme);
    if (ret != m.end() && (ret->second != std::nullopt || !obj.has_value()))
      throw RuntimeError(token, "redeclaration");
  }
  m[token.lexeme] = obj;
}

Ltype Interp::Env::get(const Token &token) const {
  auto ret = m.find(token.lexeme);
  if (ret == m.end()) {
    if (enclosing != nullptr)
      return enclosing->get(token);
    else
      throw RuntimeError(token, "undeclared variable");
  } else if (!ret->second.has_value()) {
    throw RuntimeError(token, "uninitialized variable");
  }

  return ret->second.value();
}

Ltype Interp::Env::getAt(const Token &token, std::size_t distance) {
  return ancestor(distance)->get(token);
}

Interp::Env *Interp::Env::ancestor(std::size_t distance) {
  Env *env;
  std::size_t i;

  for (i = 0, env = this; i < distance; i++)
    env = env->enclosing;
  return env;
}

void Interp::Env::assign(const Token &token, Ltype obj) {
  auto ret = m.find(token.lexeme);
  if (ret == m.end()) {
    if (enclosing != nullptr) {
      enclosing->assign(token, obj);
      return;
    } else {
      throw RuntimeError(token, "undeclared variable");
    }
  }
  ret->second = obj;
}

void Interp::Env::assignAt(const Token &token, Ltype obj,
                           std::size_t distance) {
  ancestor(distance)->assign(token, obj);
}

void Interp::visit(const StmtExpr &stmt) { eval(stmt.exprp); }

void Interp::visit(const StmtPrint &stmt) {
  fmt::print("{}\n", valueToString(eval(stmt.exprp)));
}

void Interp::visit(const StmtLoop &stmt) {
  ReclaimerCtx ctx(*this);

  while (isTruthful(ctx.add(eval(stmt.condp)))) {
    try {
      execute(*stmt.stmtp);
    } catch (Break &exception) {
      break;
    } catch (Continue &exception) {
      continue;
    }
  }
}

void Interp::visit(const StmtVar &stmt) {
  // always set variables to uninitialized before attempting to evaluate
  // the possible initializing expression. This makes weird code in global
  // scope like the following erroneous
  // var x = x;
  envp->def(stmt.token, std::nullopt);
  if (stmt.exprp != nullptr)
    envp->def(stmt.token, eval(stmt.exprp));
}

void Interp::visit(const StmtIf &stmt) {
  ReclaimerCtx ctx(*this);

  if (isTruthful(ctx.add(eval(stmt.condp))))
    execute(*stmt.thenBranch);
  else if (stmt.elseBranch != nullptr)
    execute(*stmt.elseBranch);
}

void Interp::visit(const StmtLoopFlow &stmt) {
  switch (stmt.token.type) {
  case BREAK:
    throw Interp::Break();
    break;
  case CONTINUE:
    throw Interp::Continue();
    break;
  default:
    myAssert(stmt.token, "unhandled loop flow control statement");
    break;
  }
}

void Interp::visit(std::shared_ptr<const StmtFun> stmtp) {
  ReclaimerCtx ctx(*this);
  // false -- not a ctor
  envp->def(stmtp->token, alloc<Lfunc>(ctx, stmtp, envp, false));
}

void Interp::visit(const StmtReturn &stmt) {
  Ltype retVal;

  // implicitly return nil by default
  retVal = Lnil();
  if (stmt.exprp != nullptr)
    retVal = eval(stmt.exprp);

  throw Interp::Return(retVal);
}

void Interp::visit(const StmtClass &stmt) {
  Ltype obj;
  ClassPtr superPtr;
  std::unordered_map<std::string, Lfunc *> methods, staticMethods;
  std::size_t ctorArity;
  ClassPtr cptr;
  Interp::Env *enclose;
  Env *save;

  ReclaimerCtx ctx(*this);

  ctorArity = 0;
  superPtr = nullptr;
  enclose = nullptr;

  if (stmt.superExpr != nullptr) {
    obj = ctx.add(eval(stmt.superExpr));
    if (!std::holds_alternative<ClassPtr>(obj))
      throw RuntimeError(stmt.superExpr->token,
                         "expected class, got " + typeToString(obj));

    superPtr = std::get<ClassPtr>(obj);
    enclose = alloc<Env>(ctx, envp);
    enclose->def(Token(Token::Type::SUPER, "super", "", 0), superPtr);
    save = envp;
    envp = enclose;
  }

  for (const std::shared_ptr<const StmtFun> &ptr : stmt.methods)
    methods[ptr->token.lexeme] = alloc<Lfunc>(ctx, ptr, envp, false);
  for (const std::shared_ptr<const StmtFun> &ptr : stmt.staticMethods)
    staticMethods[ptr->token.lexeme] = alloc<Lfunc>(ctx, ptr, envp, false);
  // look for optional ctor definition
  if (stmt.ctor != nullptr) {
    ctorArity = stmt.ctor->params.size();
    // true -- is a ctor
    methods[stmt.token.lexeme] = alloc<Lfunc>(ctx, stmt.ctor, envp, true);
  }

  cptr = alloc<Lclass>(ctx, stmt.token.lexeme, ctorArity, methods,
                       staticMethods, superPtr, enclose);
  if (superPtr != nullptr)
    envp = save;
  envp->def(stmt.token, cptr);
}

void Interp::handleRuntimeError(RuntimeError &ex) {
  error(ex.token, ex.what());
  hadRuntimeError = 1;
}

Interp::RuntimeError::RuntimeError(Token token, std::string what)
    : std::runtime_error(what), token(token) {}

void Interp::run(std::string inputStr) {
  std::list<Token> tokenList;
  std::list<std::shared_ptr<const Stmt>> stmtPList;

  Resolver resolver(*this);

  Scanner scanner(inputStr);

  tokenList = scanner.scanTokens();
  if (hadError)
    return;
  stmtPList = Parser::parse(tokenList);
  if (hadError)
    return;
  resolver.resolve(stmtPList);
  if (hadError)
    return;
  interpret(stmtPList);
}

void Interp::testScanner(std::string inputStr) {
  Scanner s(inputStr);

  for (Token t : s.scanTokens())
    fmt::print("{}\n", std::string(t));
}

void Interp::runPrompt() {
  std::string inputStr;

  while (1) {
    fmt::print("> ");
    std::getline(std::cin, inputStr);
    if (inputStr.size() == 0) {
      fmt::print("\n");
      break;
    }

    //auto start = std::chrono::steady_clock::now();
    run(inputStr);
    //std::chrono::duration<float> elapsed =
    //				std::chrono::steady_clock::now() - start;
    //fmt::print("Elapsed time: {:.4f}s\n", elapsed.count());
    hadError = 0;
  }
}

void Interp::runFile(std::string path) {
  std::string input;
  std::ifstream inputFileStream(path, std::ios::binary);

  if (!inputFileStream)
    throw std::runtime_error("error opening file");

  input = readAll(inputFileStream);

  //auto start = std::chrono::steady_clock::now();
  run(input);
  //testScanner(input);
  //std::chrono::duration<float> elapsed =
  //				std::chrono::steady_clock::now() - start;
  //fmt::print("Elapsed time: {:.4f}s\n", elapsed.count());
}

bool Interp::hadError;
bool Interp::hadRuntimeError;

void Interp::error(std::size_t lineNum, std::string msg) {
  report(lineNum, "", msg);
  hadError = 1;
}

void Interp::error(Token token, std::string msg) {
  report(token, msg);
  hadError = 1;
}

void Interp::report(Token token, std::string msg) {
  if (token.type == EOFF)
    report(token.lineNum, "at end", msg);
  else
    report(token.lineNum, "at '" + token.lexeme + "'", msg);
}

void Interp::report(std::size_t lineNum, std::string location,
                    std::string msg) {
  fmt::print(stderr, "line {}: location: {}: {}\n", lineNum, location, msg);
}

Interp::Interp() : envp(&global), heapSize(0) {
  envp->def(Token(Token::Type::EOFF, "clock", "clock", 0),
            Clock::get());
  //	global.isReachable = 1;
}

Interp::~Interp() { reclaim(); }

void Interp::resolve(const Expr *expr, std::size_t distance) {
  locals[expr] = distance;
}

Ltype Interp::lookupVariable(const Token &token,
                             std::shared_ptr<const Expr> expr) {
  auto distance = locals.find(expr.get());
  if (distance != locals.end())
    return envp->getAt(token, distance->second);
  return global.get(token);
}

void Interp::mark(Env *env) {
  env->isReachable = 1;
  for (auto &[_, optObj] : env->m) {
    if (optObj.has_value())
      markLtype(optObj.value());
  }
  if (env->enclosing != nullptr)
    mark(env->enclosing);
}

void Interp::mark(Lfunc *func) {
  func->isReachable = 1;
  mark(func->enclosing);
}

void Interp::mark(Lclass *lclass) {
  lclass->isReachable = 1;
  for (auto &[_, method] : lclass->methods)
    mark(method);
  for (auto &[_, staticMethod] : lclass->staticMethods)
    mark(staticMethod);
  if (lclass->base != nullptr) {
    mark(lclass->base);
    mark(lclass->superEnv);
  }
}

void Interp::mark(Linstance *obj) {
  obj->isReachable = 1;
  for (auto &[_, lobj] : obj->properties)
    markLtype(lobj);
  mark(obj->lclass);
}

void Interp::markLtype(Ltype &l) {
  std::visit(
      [this](auto x) -> void {
        using T = decltype(x);

        if constexpr (std::is_convertible_v<T, RunEnt *>) {
          if (!x->isReachable)
            mark(x);
        }
      },
      l);
}

void Interp::reclaim() {
  //	std::size_t previousAllocatedCount, previousHeapSize;

  //	previousAllocatedCount = traced.size();
  //	previousHeapSize = heapSize;

  auto it = traced.begin();
  auto prev = it;
  while (it != traced.end()) {
    prev = it++;
    std::visit(
        [this, prev](auto y) -> void {
          if (y->isReachable)
            return;
          delete y;
          heapSize -= sizeof(std::remove_pointer_t<decltype(y)>);
          traced.erase(prev);
        },
        *prev);
  }
  //	fmt::print(stderr, "reclaimed: {}, {}\n",
  //					previousAllocatedCount - traced.size(),
  //						previousHeapSize - heapSize);
}

void Interp::unmark() {
  for (auto x : traced) {
    std::visit([](auto y) -> void { y->unmarkSelf(); }, x);
  }
}
