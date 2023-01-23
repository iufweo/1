#pragma once
#include <list>
#include <memory>

#include "expr.hpp"
#include "stmt_visitor_fwd.hpp"
#include "token.hpp"
#include "uncopyable.hpp"
// needed only for StmtFun, which is pointed to by std::shared_ptr<Stmt or
// StmtFun or Functional>
struct Stmt : public StructUncopyable,
              public std::enable_shared_from_this<Stmt> {
  Stmt() = default;
  virtual ~Stmt() = default;

  virtual void accept(StmtVisitor& v) const = 0;
};

struct StmtExpr : public Stmt {
  std::shared_ptr<const Expr> exprp;

  StmtExpr(std::shared_ptr<const Expr> exprp);
  ~StmtExpr();

  StmtExpr() = delete;

  void accept(StmtVisitor& v) const final;
};

struct StmtPrint : public Stmt {
  std::shared_ptr<const Expr> exprp;

  StmtPrint(std::shared_ptr<const Expr> exprp);
  ~StmtPrint();

  StmtPrint() = delete;

  void accept(StmtVisitor& v) const final;
};

struct StmtVar : public Stmt {
  const Token token;
  std::shared_ptr<const Expr> exprp;

  StmtVar(Token token, std::shared_ptr<const Expr> exprp);
  ~StmtVar();

  StmtVar() = delete;

  void accept(StmtVisitor& v) const final;
};

struct StmtIf : public Stmt {
  std::shared_ptr<const Expr> condp;
  const Stmt *thenBranch, *elseBranch;

  StmtIf(std::shared_ptr<const Expr> condp,
         const Stmt* thenBranch,
         const Stmt* elseBranch);
  ~StmtIf();

  StmtIf() = delete;

  void accept(StmtVisitor& v) const final;
};

struct StmtLoop : public Stmt {
  std::shared_ptr<const Expr> condp;
  // pointer is needed for implementing continue only in "for" loops
  std::shared_ptr<const Expr> exprp;
  const Stmt* body;

  StmtLoop(std::shared_ptr<const Expr> condp,
           std::shared_ptr<const Expr> exprp,
           const Stmt* stmtp);
  ~StmtLoop();

  StmtLoop() = delete;

  void accept(StmtVisitor& v) const final;
};

struct StmtList : public Stmt {
  const std::list<std::shared_ptr<const Stmt>> stmts;

  StmtList(std::list<std::shared_ptr<const Stmt>>&& stms);
  ~StmtList() = default;

  StmtList() = delete;

  void accept(StmtVisitor& v) const final;
};

struct StmtLoopFlow : public Stmt {
  const Token token;

  StmtLoopFlow(Token token);
  ~StmtLoopFlow() = default;

  StmtLoopFlow() = delete;

  void accept(StmtVisitor& v) const final;
};

struct StmtReturn : public Stmt {
  const Token token;
  const std::shared_ptr<const Expr> exprp;

  StmtReturn(Token token, std::shared_ptr<const Expr>&& exprp);
  ~StmtReturn() = default;

  StmtReturn() = delete;

  void accept(StmtVisitor& v) const final;
};

struct StmtFun : public Stmt, public Functional {
  const Token token;

  StmtFun(Token token,
          std::list<Token> params,
          std::unique_ptr<const StmtList>&& listp);
  ~StmtFun() = default;
  StmtFun() = delete;

  void accept(StmtVisitor& v) const final;
};

struct StmtClass : public Stmt {
  const Token token;
  const std::shared_ptr<const ExprVar> superExpr;
  const std::shared_ptr<const StmtFun> ctor;
  const std::list<std::shared_ptr<const StmtFun>> methods;
  const std::list<std::shared_ptr<const StmtFun>> staticMethods;

  StmtClass(Token token,
            std::shared_ptr<const ExprVar>&& superExpr,
            std::shared_ptr<const StmtFun> ctor,
            std::list<std::shared_ptr<const StmtFun>>&& methods,
            std::list<std::shared_ptr<const StmtFun>>&& staticMethods);
  ~StmtClass() = default;

  StmtClass() = delete;

  void accept(StmtVisitor& v) const final;
};
