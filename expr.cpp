#include "expr_visitor.hpp"

#include "expr.hpp"

ExprBinary::ExprBinary(std::shared_ptr<const Expr> left,
                       Token oper,
                       std::shared_ptr<const Expr> right)
    : left(left), oper(oper), right(right) {}

Ltype ExprBinary::accept(ExprVisitor& v) const {
  return v.visit(this);
}

ExprGrouping::ExprGrouping(std::shared_ptr<const Expr> exprp) : exprp(exprp) {}

Ltype ExprGrouping::accept(ExprVisitor& v) const {
  return v.visit(this);
}

ExprLiteral::ExprLiteral(Ltype value) : value(value) {}

Ltype ExprLiteral::accept(ExprVisitor& v) const {
  return v.visit(this);
}

ExprUnary::ExprUnary(Token oper, std::shared_ptr<const Expr> exprp)
    : oper(oper), exprp(exprp) {}

Ltype ExprUnary::accept(ExprVisitor& v) const {
  return v.visit(this);
}

ExprTern::ExprTern(std::shared_ptr<const Expr> cond,
                   std::shared_ptr<const Expr> thenp,
                   std::shared_ptr<const Expr> elsep)
    : cond(cond), thenp(thenp), elsep(elsep) {}

Ltype ExprTern::accept(ExprVisitor& v) const {
  return v.visit(this);
}

ExprVar::ExprVar(Token token) : token(token) {}

Ltype ExprVar::accept(ExprVisitor& v) const {
  return v.visit(this);
}

ExprAssign::ExprAssign(Token token, std::shared_ptr<const Expr> exprp)
    : token(token), exprp(exprp) {}

Ltype ExprAssign::accept(ExprVisitor& v) const {
  return v.visit(this);
}

ExprCall::ExprCall(std::shared_ptr<const Expr> exprp,
                   Token savedParen,
                   std::list<std::shared_ptr<const Expr>>&& args)
    : exprp(exprp), savedParen(savedParen), args(std::move(args)) {}

Ltype ExprCall::accept(ExprVisitor& v) const {
  return v.visit(this);
}

ExprGet::ExprGet(std::shared_ptr<const Expr> exprp, Token token)
    : exprp(exprp), token(token) {}

Ltype ExprGet::accept(ExprVisitor& v) const {
  return v.visit(this);
}

ExprSet::ExprSet(std::shared_ptr<const ExprGet> get,
                 Token token,
                 std::shared_ptr<const Expr> exprp)
    : get(get), token(token), exprp(exprp) {}

Ltype ExprSet::accept(ExprVisitor& v) const {
  return v.visit(this);
}

ExprThis::ExprThis(Token token) : token(token) {}

Ltype ExprThis::accept(ExprVisitor& v) const {
  return v.visit(this);
}

ExprSuper::ExprSuper(Token token, Token method)
    : token(token), method(method) {}

Ltype ExprSuper::accept(ExprVisitor& v) const {
  return v.visit(this);
}

ExprComma::ExprComma(std::shared_ptr<const Expr> left,
                     Token oper,
                     std::shared_ptr<const Expr> right)
    : left(left), oper(oper), right(right) {}

Ltype ExprComma::accept(ExprVisitor& v) const {
  return v.visit(this);
}

ExprLogical::ExprLogical(std::shared_ptr<const Expr> left,
                         Token oper,
                         std::shared_ptr<const Expr> right)
    : left(left), oper(oper), right(right) {}

Ltype ExprLogical::accept(ExprVisitor& v) const {
  return v.visit(this);
}

ExprFun::ExprFun(std::list<Token> params,
                 std::unique_ptr<const StmtList>&& listp)
    : Functional(params, std::move(listp)) {}

Ltype ExprFun::accept(ExprVisitor& v) const {
  return v.visit(std::static_pointer_cast<const ExprFun>(shared_from_this()));
}
