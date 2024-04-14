#pragma once
#include <list>
#include <memory>

#include "expr_visitor_fwd.hpp"
#include "functional.hpp"
#include "ltype.hpp"
#include "stmt_fwd.hpp"
#include "token.hpp"
#include "uncopyable.hpp"
// for ExprFun only
struct Expr : public Uncopyable, public std::enable_shared_from_this<Expr> {
  Expr() = default;
  virtual ~Expr() = default;

  virtual Ltype accept(ExprVisitor& v) const = 0;
};

struct ExprBinary : public Expr {
  const std::shared_ptr<const Expr> left;
  const Token oper;
  const std::shared_ptr<const Expr> right;

  ExprBinary(std::shared_ptr<const Expr> left,
             Token oper,
             std::shared_ptr<const Expr> right);

  ExprBinary() = delete;

  Ltype accept(ExprVisitor& v) const final;
};

struct ExprGrouping : public Expr {
  const std::shared_ptr<const Expr> exprp;

  ExprGrouping(std::shared_ptr<const Expr> exprp);

  ExprGrouping() = delete;

  Ltype accept(ExprVisitor& v) const final;
};

struct ExprLiteral : public Expr {
  const Ltype value;

  ExprLiteral(Ltype value);

  ExprLiteral() = delete;

  Ltype accept(ExprVisitor& v) const final;
};

struct ExprUnary : public Expr {
  const Token oper;
  const std::shared_ptr<const Expr> exprp;

  ExprUnary(Token oper, const std::shared_ptr<const Expr> exprp);

  ExprUnary() = delete;

  Ltype accept(ExprVisitor& v) const final;
};

struct ExprTern : public Expr {
  const std::shared_ptr<const Expr> cond;
  const std::shared_ptr<const Expr> thenp;
  const std::shared_ptr<const Expr> elsep;

  ExprTern(std::shared_ptr<const Expr> cond,
           std::shared_ptr<const Expr> thenp,
           std::shared_ptr<const Expr> elsep);

  ExprTern() = delete;

  Ltype accept(ExprVisitor& v) const final;
};

struct ExprVar : public Expr {
  const Token token;

  ExprVar(Token token);

  ExprVar() = delete;

  Ltype accept(ExprVisitor& v) const final;
};

struct ExprAssign : public Expr {
  const Token token;
  const std::shared_ptr<const Expr> exprp;

  ExprAssign(Token token, std::shared_ptr<const Expr> exprp);

  ExprAssign() = delete;

  Ltype accept(ExprVisitor& v) const final;
};

struct ExprCall : public Expr {
  const std::shared_ptr<const Expr> exprp;
  const Token savedParen;
  const std::list<std::shared_ptr<const Expr>> args;

  ExprCall(std::shared_ptr<const Expr> exprp,
           Token savedParen,
           std::list<std::shared_ptr<const Expr>>&& args);

  ExprCall() = delete;

  Ltype accept(ExprVisitor& v) const final;
};

struct ExprGet : public Expr {
  const std::shared_ptr<const Expr> exprp;
  const Token token;

  ExprGet(std::shared_ptr<const Expr> exprp, Token token);

  ExprGet() = delete;

  Ltype accept(ExprVisitor& v) const final;
};

struct ExprSet : public Expr {
  const std::shared_ptr<const ExprGet> get;
  const Token token;
  const std::shared_ptr<const Expr> exprp;

  ExprSet(std::shared_ptr<const ExprGet> get,
          Token token,
          std::shared_ptr<const Expr> exprp);

  ExprSet() = delete;

  Ltype accept(ExprVisitor& v) const final;
};

struct ExprThis : public Expr {
  const Token token;

  ExprThis(Token token);

  ExprThis() = delete;

  Ltype accept(ExprVisitor& v) const final;
};

struct ExprSuper : public Expr {
  const Token token;
  const Token method;

  ExprSuper(Token token, Token method);

  ExprSuper() = delete;

  Ltype accept(ExprVisitor& v) const final;
};

struct ExprComma : public Expr {
  const std::shared_ptr<const Expr> left;
  const Token oper;
  const std::shared_ptr<const Expr> right;

  ExprComma(std::shared_ptr<const Expr> left,
            Token oper,
            std::shared_ptr<const Expr> right);

  ExprComma() = delete;

  Ltype accept(ExprVisitor& v) const final;
};

struct ExprLogical : public Expr {
  const std::shared_ptr<const Expr> left;
  const Token oper;
  const std::shared_ptr<const Expr> right;

  ExprLogical(std::shared_ptr<const Expr> left,
              Token oper,
              std::shared_ptr<const Expr> right);

  ExprLogical() = delete;

  Ltype accept(ExprVisitor& v) const final;
};

struct ExprFun : public Expr, public Functional {
  ExprFun() = delete;
  ExprFun(std::list<Token> params, std::unique_ptr<const StmtList>&& listp);

  Ltype accept(ExprVisitor& v) const final;
};
