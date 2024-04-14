#pragma once
#include <memory>

#include "expr_fwd.hpp"
#include "ltype.hpp"
#include "uncopyable.hpp"

class ExprVisitor : public Uncopyable {
 public:
  virtual Ltype visit(const ExprBinary* expr) = 0;
  virtual Ltype visit(const ExprGrouping* expr) = 0;
  virtual Ltype visit(const ExprLiteral* expr) = 0;
  virtual Ltype visit(const ExprUnary* expr) = 0;
  virtual Ltype visit(const ExprTern* expr) = 0;
  virtual Ltype visit(const ExprVar* expr) = 0;
  virtual Ltype visit(const ExprAssign* expr) = 0;
  virtual Ltype visit(const ExprCall* expr) = 0;
  virtual Ltype visit(const ExprGet* expr) = 0;
  virtual Ltype visit(const ExprSet* expr) = 0;
  virtual Ltype visit(const ExprThis* expr) = 0;
  virtual Ltype visit(const ExprSuper* expr) = 0;
  virtual Ltype visit(const ExprComma* expr) = 0;
  virtual Ltype visit(const ExprLogical* expr) = 0;
  virtual Ltype visit(std::shared_ptr<const ExprFun> expr) = 0;

  virtual ~ExprVisitor() = default;
};
