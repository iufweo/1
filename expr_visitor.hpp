#pragma once
#include "expr.hpp"

class ExprVisitor : public ClassUncopyable {
public:
  virtual Ltype visit(std::shared_ptr<const ExprBinary> expr) = 0;
  virtual Ltype visit(std::shared_ptr<const ExprGrouping> expr) = 0;
  virtual Ltype visit(std::shared_ptr<const ExprLiteral> expr) = 0;
  virtual Ltype visit(std::shared_ptr<const ExprUnary> expr) = 0;
  virtual Ltype visit(std::shared_ptr<const ExprTern> expr) = 0;
  virtual Ltype visit(std::shared_ptr<const ExprVar> expr) = 0;
  virtual Ltype visit(std::shared_ptr<const ExprAssign> expr) = 0;
  virtual Ltype visit(std::shared_ptr<const ExprCall> expr) = 0;
  virtual Ltype visit(std::shared_ptr<const ExprGet> expr) = 0;
  virtual Ltype visit(std::shared_ptr<const ExprSet> expr) = 0;
  virtual Ltype visit(std::shared_ptr<const ExprThis> expr) = 0;
  virtual Ltype visit(std::shared_ptr<const ExprSuper> expr) = 0;
  virtual Ltype visit(std::shared_ptr<const ExprComma> expr) = 0;
  virtual Ltype visit(std::shared_ptr<const ExprLogical> expr) = 0;
  virtual Ltype visit(std::shared_ptr<const ExprFun> expr) = 0;

  virtual ~ExprVisitor() = default;
};
