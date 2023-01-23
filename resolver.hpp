#pragma once
#include <list>
#include <memory>
#include <unordered_map>

#include "expr_visitor.hpp"
#include "ltype.hpp"
#include "stmt_visitor.hpp"
#include "token.hpp"

// forward
class Interp;

class Resolver : public ExprVisitor, public StmtVisitor {
 private:
  void visit(const StmtExpr& stmt) final;
  void visit(const StmtPrint& stmt) final;
  void visit(const StmtVar& stmt) final;
  void visit(const StmtLoop& stmt) final;
  void visit(const StmtIf& stmt) final;
  void visit(const StmtList& stmt) final;
  void visit(const StmtReturn& stmt) final;
  void visit(const StmtLoopFlow& stmt) final;
  void visit(std::shared_ptr<const StmtFun> stmtp) final;
  void visit(const StmtClass& stmt) final;
  void resolveFunctional(const Functional& fun);

  Ltype visit(const ExprBinary* expr) final;
  Ltype visit(const ExprGrouping* expr) final;
  Ltype visit(const ExprLiteral* expr) final;
  Ltype visit(const ExprUnary* expr) final;
  Ltype visit(const ExprTern* expr) final;
  Ltype visit(const ExprVar* expr) final;
  Ltype visit(const ExprAssign* expr) final;
  Ltype visit(const ExprCall* expr) final;
  Ltype visit(const ExprGet* expr) final;
  Ltype visit(const ExprSet* expr) final;
  Ltype visit(const ExprThis* expr) final;
  Ltype visit(const ExprSuper* expr) final;
  Ltype visit(const ExprComma* expr) final;
  Ltype visit(const ExprLogical* expr) final;
  Ltype visit(std::shared_ptr<const ExprFun> expr) final;

  enum class VarState { DECL, SET, READ };

  std::list<std::unordered_map<Token, VarState, Token::Hash>> scopes;
  void beginScope();
  void endScope();
  void declare(Token token);
  void initialize(Token token);

 public:
  enum class ScopeType : int {
    NONE = 0,
    LOOP = 1,
    FUNC = 2,
    CLASS = 4,
    METHOD = 8,
    CTOR = 16,
    STATIC_METHOD = 32,
    SUBCLASS = 64
  };

 private:
  ScopeType currentScopeType;
  bool isScopeType(ScopeType s) const;

  class ExchangeScopeTypes {
   private:
    Resolver& resolver;
    ScopeType save;

   public:
    ExchangeScopeTypes(Resolver& resolver);

    void add(ScopeType s);

    ~ExchangeScopeTypes();
  };

  Ltype resolve(std::shared_ptr<const Expr> expr);
  void resolve(const Stmt& stmt);
  std::optional<decltype(Resolver::scopes)::value_type::iterator> resolveLocal(
      const Expr* expr,
      const Token& token);
  void initializeAt(Token token, std::size_t distance);

  Interp& interp;

 public:
  void resolve(const std::list<std::shared_ptr<const Stmt>>& list);

  Resolver(Interp& interp);
};
