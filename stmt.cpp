#include "expr.hpp"
#include "stmt_visitor.hpp"

#include "functional.hpp"
#include "stmt.hpp"

StmtExpr::StmtExpr(std::shared_ptr<const Expr> exprp) : exprp(exprp) {}

StmtExpr::~StmtExpr() {}

void StmtExpr::accept(StmtVisitor& v) const {
  v.visit(*this);
}

StmtPrint::StmtPrint(std::shared_ptr<const Expr> exprp) : exprp(exprp) {}

StmtPrint::~StmtPrint() {}

void StmtPrint::accept(StmtVisitor& v) const {
  v.visit(*this);
}

StmtVar::StmtVar(Token token, std::shared_ptr<const Expr> exprp)
    : token(token), exprp(exprp) {}

StmtVar::~StmtVar() {}

void StmtVar::accept(StmtVisitor& v) const {
  v.visit(*this);
}

StmtList::StmtList(std::list<std::shared_ptr<const Stmt>>&& stms)
    : stmts(std::move(stms)) {}

void StmtList::accept(StmtVisitor& v) const {
  v.visit(*this);
}

StmtIf::StmtIf(std::shared_ptr<const Expr> condp,
               const Stmt* thenBranch,
               const Stmt* elseBranch)
    : condp(condp), thenBranch(thenBranch), elseBranch(elseBranch) {}

StmtIf::~StmtIf() {
  delete thenBranch;
  delete elseBranch;
}

void StmtIf::accept(StmtVisitor& v) const {
  v.visit(*this);
}

StmtLoop::StmtLoop(std::shared_ptr<const Expr> condp,
                   std::shared_ptr<const Expr> exprp,
                   const Stmt* stmtp)
    : condp(condp), exprp(exprp), body(stmtp) {}

StmtLoop::~StmtLoop() {
  delete body;
}

void StmtLoop::accept(StmtVisitor& v) const {
  v.visit(*this);
}

StmtLoopFlow::StmtLoopFlow(Token token) : token(token) {}

void StmtLoopFlow::accept(StmtVisitor& v) const {
  v.visit(*this);
}

StmtFun::StmtFun(Token token,
                 std::list<Token> params,
                 std::unique_ptr<const StmtList>&& listp)
    : Functional(params, std::move(listp)), token(token) {}

void StmtFun::accept(StmtVisitor& v) const {
  v.visit(std::static_pointer_cast<const StmtFun>(shared_from_this()));
}

StmtClass::StmtClass(Token token,
                     std::shared_ptr<const ExprVar>&& superExpr,
                     std::shared_ptr<const StmtFun> ctor,
                     std::list<std::shared_ptr<const StmtFun>>&& methods,
                     std::list<std::shared_ptr<const StmtFun>>&& staticMethods)
    : token(token),
      superExpr(std::move(superExpr)),
      ctor(ctor),
      methods(std::move(methods)),
      staticMethods(std::move(staticMethods)) {}

void StmtClass::accept(StmtVisitor& v) const {
  v.visit(*this);
}

StmtReturn::StmtReturn(Token token, std::shared_ptr<const Expr>&& exprp)
    : token(token), exprp(std::move(exprp)) {}

void StmtReturn::accept(StmtVisitor& v) const {
  v.visit(*this);
}
