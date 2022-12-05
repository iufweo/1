#include "expr_visitor.hpp"

#include "expr.hpp"

ExprBinary::ExprBinary(std::shared_ptr<const Expr> left, Token oper,
                       std::shared_ptr<const Expr> right)
    : left(left), oper(oper), right(right) {}

ExprBinary::~ExprBinary() {
  //	delete left;
  //	delete right;
}

Ltype ExprBinary::accept(ExprVisitor &v) const {
  return v.visit(
      std::static_pointer_cast<const ExprBinary>(shared_from_this()));
}

ExprGrouping::ExprGrouping(std::shared_ptr<const Expr> exprp) : exprp(exprp) {}

ExprGrouping::~ExprGrouping() {
  //	delete exprp;
}

Ltype ExprGrouping::accept(ExprVisitor &v) const {
  return v.visit(
      std::static_pointer_cast<const ExprGrouping>(shared_from_this()));
}

ExprLiteral::ExprLiteral(Ltype value) : value(value) {}

Ltype ExprLiteral::accept(ExprVisitor &v) const {
  return v.visit(
      std::static_pointer_cast<const ExprLiteral>(shared_from_this()));
}

ExprUnary::ExprUnary(Token oper, std::shared_ptr<const Expr> exprp)
    : oper(oper), exprp(exprp) {}

ExprUnary::~ExprUnary() {
  //	delete exprp;
}

Ltype ExprUnary::accept(ExprVisitor &v) const {
  return v.visit(std::static_pointer_cast<const ExprUnary>(shared_from_this()));
}

ExprTern::ExprTern(std::shared_ptr<const Expr> cond,
                   std::shared_ptr<const Expr> thenp,
                   std::shared_ptr<const Expr> elsep)
    : cond(cond), thenp(thenp), elsep(elsep) {}

ExprTern::~ExprTern() {
  //	delete cond;
  //	delete thenp;
  //	delete elsep;
}

Ltype ExprTern::accept(ExprVisitor &v) const {
  return v.visit(std::static_pointer_cast<const ExprTern>(shared_from_this()));
}

ExprVar::ExprVar(Token token) : token(token) {}

Ltype ExprVar::accept(ExprVisitor &v) const {
  return v.visit(std::static_pointer_cast<const ExprVar>(shared_from_this()));
}

ExprAssign::ExprAssign(Token token, std::shared_ptr<const Expr> exprp)
    : token(token), exprp(exprp) {}

ExprAssign::~ExprAssign() {
  //	delete exprp;
}

Ltype ExprAssign::accept(ExprVisitor &v) const {
  return v.visit(
      std::static_pointer_cast<const ExprAssign>(shared_from_this()));
}

ExprCall::ExprCall(std::shared_ptr<const Expr> exprp, Token savedParen,
                   std::list<std::shared_ptr<const Expr>> &&args)
    : exprp(exprp), savedParen(savedParen), args(std::move(args)) {}

ExprCall::~ExprCall() {
  //	delete exprp;
}

Ltype ExprCall::accept(ExprVisitor &v) const {
  return v.visit(std::static_pointer_cast<const ExprCall>(shared_from_this()));
}

ExprGet::ExprGet(std::shared_ptr<const Expr> exprp, Token token)
    : exprp(exprp), token(token) {}

ExprGet::~ExprGet() {
  //	delete exprp;
}

Ltype ExprGet::accept(ExprVisitor &v) const {
  return v.visit(std::static_pointer_cast<const ExprGet>(shared_from_this()));
}

ExprSet::ExprSet(std::shared_ptr<const ExprGet> get, Token token,
                 std::shared_ptr<const Expr> exprp)
    : get(get), token(token), exprp(exprp) {}

ExprSet::~ExprSet() {
  //	delete get;
  //	delete exprp;
}

Ltype ExprSet::accept(ExprVisitor &v) const {
  return v.visit(std::static_pointer_cast<const ExprSet>(shared_from_this()));
}

ExprThis::ExprThis(Token token) : token(token) {}

Ltype ExprThis::accept(ExprVisitor &v) const {
  return v.visit(std::static_pointer_cast<const ExprThis>(shared_from_this()));
}

ExprSuper::ExprSuper(Token token, Token method)
    : token(token), method(method) {}

Ltype ExprSuper::accept(ExprVisitor &v) const {
  return v.visit(std::static_pointer_cast<const ExprSuper>(shared_from_this()));
}

ExprComma::ExprComma(std::shared_ptr<const Expr> left, Token oper,
                     std::shared_ptr<const Expr> right)
    : left(left), oper(oper), right(right) {}

ExprComma::~ExprComma() {
  //	delete left;
  //	delete right;
}

Ltype ExprComma::accept(ExprVisitor &v) const {
  return v.visit(std::static_pointer_cast<const ExprComma>(shared_from_this()));
}

ExprLogical::ExprLogical(std::shared_ptr<const Expr> left, Token oper,
                         std::shared_ptr<const Expr> right)
    : left(left), oper(oper), right(right) {}

ExprLogical::~ExprLogical() {
  //	delete left;
  //	delete right;
}

Ltype ExprLogical::accept(ExprVisitor &v) const {
  return v.visit(
      std::static_pointer_cast<const ExprLogical>(shared_from_this()));
}

ExprFun::ExprFun(std::list<Token> params,
                 std::unique_ptr<const StmtList> &&listp)
    : Functional(params, std::move(listp)) {}

Ltype ExprFun::accept(ExprVisitor &v) const {
  return v.visit(std::static_pointer_cast<const ExprFun>(shared_from_this()));
}
