#pragma once
#include <memory>

#include "stmt.hpp"

class StmtVisitor : public ClassUncopyable {
 public:
  virtual void visit(const StmtExpr& stmt) = 0;
  virtual void visit(const StmtPrint& stmt) = 0;
  virtual void visit(const StmtVar& stmt) = 0;
  virtual void visit(const StmtLoop& stmt) = 0;
  virtual void visit(const StmtIf& stmt) = 0;
  virtual void visit(const StmtList& stmt) = 0;
  virtual void visit(const StmtReturn& stmt) = 0;
  virtual void visit(const StmtLoopFlow& stmt) = 0;
  virtual void visit(std::shared_ptr<const StmtFun> stmtp) = 0;
  virtual void visit(const StmtClass& stmt) = 0;

  virtual ~StmtVisitor() = default;
};
