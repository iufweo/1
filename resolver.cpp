#include <list>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include "expr.hpp"
#include "interp.hpp"
#include "ltype.hpp"
#include "stmt.hpp"

#include "resolver.hpp"

using enum Resolver::ScopeType;

bool Resolver::isScopeType(ScopeType s) const {
  return (int)currentScopeType & (int)s;
}

Resolver::ExchangeScopeTypes::ExchangeScopeTypes(Resolver& resolver)
    : resolver(resolver), save(resolver.currentScopeType) {}

Resolver::ScopeType operator|(Resolver::ScopeType lhs,
                              Resolver::ScopeType rhs) {
  return (Resolver::ScopeType)((int)lhs | (int)rhs);
}

void Resolver::ExchangeScopeTypes::add(ScopeType s) {
  resolver.currentScopeType = resolver.currentScopeType | s;
}

Resolver::ExchangeScopeTypes::~ExchangeScopeTypes() {
  resolver.currentScopeType = save;
}

Ltype Resolver::visit(const ExprBinary* expr) {
  resolve(expr->left);
  resolve(expr->right);
  return Lnil();
}

Ltype Resolver::visit(const ExprComma* expr) {
  resolve(expr->left);
  resolve(expr->right);
  return Lnil();
}

Ltype Resolver::visit(const ExprLogical* expr) {
  resolve(expr->left);
  resolve(expr->right);
  return Lnil();
}

Ltype Resolver::visit(const ExprGrouping* expr) {
  resolve(expr->exprp);
  return Lnil();
}

Ltype Resolver::visit(const ExprLiteral* expr) {
  static_cast<void>(expr);
  return Lnil();
}

Ltype Resolver::visit(const ExprUnary* expr) {
  resolve(expr->exprp);
  return Lnil();
}

Ltype Resolver::visit(const ExprTern* expr) {
  resolve(expr->cond);
  resolve(expr->thenp);
  resolve(expr->elsep);
  return Lnil();
}

Ltype Resolver::visit(const ExprVar* expr) {
  auto optIt = resolveLocal(expr, expr->token);
  if (!scopes.empty()) {
    if (optIt.has_value() && optIt.value()->second == VarState::DECL)
      Interp::error(expr->token,
                    "static: "
                    "uninitialized variable");
    // if no optional iterator, assume it's defined and global
    // even though it might be not, leaving the task to the
    // interpreter
  }
  return Lnil();
}

Ltype Resolver::visit(const ExprAssign* expr) {
  resolve(expr->exprp);

  auto optIt = resolveLocal(expr, expr->token);
  if (optIt.has_value() && optIt.value()->second != VarState::READ)
    optIt.value()->second = VarState::SET;
  return Lnil();
}

Ltype Resolver::visit(const ExprCall* expr) {
  resolve(expr->exprp);
  for (const auto& ptr : expr->args)
    resolve(ptr);
  return Lnil();
}

Ltype Resolver::visit(const ExprGet* expr) {
  resolve(expr->exprp);
  return Lnil();
}

Ltype Resolver::visit(const ExprSet* expr) {
  resolve(expr->get);
  resolve(expr->exprp);
  return Lnil();
}

Ltype Resolver::visit(const ExprThis* expr) {
  if (!isScopeType(METHOD))
    Interp::error(expr->token, "outside method scope");
  if (isScopeType(STATIC_METHOD))
    Interp::error(expr->token, "in static method");
  resolveLocal(expr, expr->token);
  return Lnil();
}

Ltype Resolver::visit(const ExprSuper* expr) {
  if (!isScopeType(METHOD))
    Interp::error(expr->token, "outside method scope");
  else if (!isScopeType(SUBCLASS))
    Interp::error(expr->token, "class does not have an ancestor");
  resolveLocal(expr, expr->token);
  return Lnil();
}

void Resolver::visit(const StmtExpr& stmt) {
  resolve(stmt.exprp);
}

void Resolver::visit(const StmtPrint& stmt) {
  resolve(stmt.exprp);
}

void Resolver::visit(const StmtVar& stmt) {
  declare(stmt.token);
  if (stmt.exprp != nullptr) {
    resolve(stmt.exprp);
    initialize(stmt.token);
  }
}

void Resolver::visit(const StmtLoop& stmt) {
  ExchangeScopeTypes once(*this);

  once.add(LOOP);
  // may contain variable definition used in the conditional expression
  // for (var x; ...
  resolve(*stmt.body);
  if (stmt.exprp != nullptr)
    resolve(stmt.exprp);
  resolve(stmt.condp);
}

void Resolver::visit(const StmtIf& stmt) {
  resolve(stmt.condp);
  resolve(*stmt.thenBranch);
  if (stmt.elseBranch != nullptr)
    resolve(*stmt.elseBranch);
}

void Resolver::visit(const StmtList& stmt) {
  beginScope();
  resolve(stmt.stmts);
  endScope();
}

void Resolver::visit(const StmtReturn& stmt) {
  if (!isScopeType(FUNC))
    Interp::error(stmt.token, "outside function scope");
  if (isScopeType(CTOR) && stmt.exprp != nullptr)
    Interp::error(stmt.token,
                  "returning a"
                  " value inside constructor");
  if (stmt.exprp != nullptr)
    resolve(stmt.exprp);
}

void Resolver::visit(const StmtLoopFlow& stmt) {
  if (!isScopeType(LOOP))
    Interp::error(stmt.token, "outside loop scope");
}

void Resolver::visit(const StmtClass& stmt) {
  ExchangeScopeTypes once(*this);

  declare(stmt.token);
  initialize(stmt.token);

  if (stmt.superExpr != nullptr) {
    if (stmt.superExpr->token.lexeme == stmt.token.lexeme)
      Interp::error(stmt.superExpr->token, "inherits from itself");
    else
      resolve(stmt.superExpr);
  }

  once.add(FUNC | METHOD | CLASS);
  if (stmt.superExpr != nullptr) {
    once.add(SUBCLASS);
    beginScope();
    (scopes.back())[Token(Token::Type::SUPER, "super", "", 0)] = VarState::READ;
  }
  beginScope();
  (scopes.back())[Token(Token::Type::THIS, "this", "", 0)] = VarState::READ;
  if (stmt.ctor != nullptr) {
    ExchangeScopeTypes twice(*this);
    twice.add(CTOR);
    resolveFunctional(*stmt.ctor);
  }
  for (const auto& ptr : stmt.methods)
    resolveFunctional(*ptr);
  endScope();

  once.add(STATIC_METHOD);
  for (const auto& ptr : stmt.staticMethods) {
    if (ptr->token.lexeme == stmt.token.lexeme)
      Interp::error(ptr->token, "constructor defined as a static method");
    resolveFunctional(*ptr);
  }
  if (stmt.superExpr != nullptr)
    endScope();
}

void Resolver::resolve(const std::list<std::shared_ptr<const Stmt>>& list) {
  for (const auto& ptr : list)
    resolve(*ptr);
}

Resolver::Resolver(Interp& interp) : currentScopeType(NONE), interp(interp) {}

void Resolver::resolve(const Stmt& stmt) {
  stmt.accept(*this);
}

Ltype Resolver::resolve(std::shared_ptr<const Expr> expr) {
  expr->accept(*this);
  return Lnil();
}

void Resolver::resolveFunctional(const Functional& fun) {
  ExchangeScopeTypes once(*this);

  once.add(FUNC);
  beginScope();
  for (const auto& token : fun.params) {
    declare(token);
    initialize(token);
  }

  resolve(fun.listp->stmts);
  endScope();
}

Ltype Resolver::visit(std::shared_ptr<const ExprFun> expr) {
  resolveFunctional(*expr);
  return Lnil();
}

void Resolver::visit(std::shared_ptr<const StmtFun> stmtp) {
  declare(stmtp->token);
  initialize(stmtp->token);
  resolveFunctional(*stmtp);
}

void Resolver::beginScope() {
  scopes.push_back(std::unordered_map<Token, VarState, Token::Hash>{});
}

void Resolver::endScope() {
  for (const auto& [token, state] : scopes.back()) {
    switch (state) {
      case VarState::DECL:
        Interp::report(token, "declared but not used");
        break;
      case VarState::SET:
        Interp::report(token, "set but never used");
        break;
      case VarState::READ:
        break;
    }
  }
  scopes.pop_back();
}

void Resolver::declare(const Token& token) {
  // not global
  if (!scopes.empty()) {
    auto it = scopes.back().find(token);
    if (it != scopes.back().end()) {
      Interp::error(token,
                    "static: "
                    "redeclaration in non-global scope");
      Interp::error(it->first,
                    "static: "
                    "previously declared here");
    }
    (scopes.back())[token] = VarState::DECL;
  }
}

void Resolver::initialize(const Token& token) {
  if (!scopes.empty())
    (scopes.back())[token] = VarState::SET;
}

std::optional<decltype(Resolver::scopes)::value_type::iterator>
Resolver::resolveLocal(const Expr* expr, const Token& token) {
  std::size_t count;
  auto sb = scopes.rbegin();
  auto se = scopes.rend();

  // leaves names in global uninspected
  for (count = 0; sb != se; count++, sb++) {
    auto it = sb->find(token);
    if (it != sb->end()) {
      // methods are always SET, thus they are always READ
      if (it->second == VarState::SET)
        it->second = VarState::READ;
      interp.resolve(expr, count);
      return it;
    }
  }

  return std::nullopt;
}
